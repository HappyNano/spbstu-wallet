#include "psql_transaction.h"

#include <spdlog/spdlog.h>

using namespace cxx;

PsqlTransaction::PsqlTransaction(std::unique_ptr< pqxx::work > txn)
  : txn_{ std::move(txn) } {
}

PsqlTransaction::~PsqlTransaction() {
    if (txn_) {
        txn_->commit();
    }
}

void PsqlTransaction::abort() {
    if (txn_) {
        txn_->abort();
        txn_.reset();
    } else {
        SPDLOG_ERROR("Failed to abort transcation. Transcation is closed");
    }
}

void PsqlTransaction::commit() {
    if (txn_) {
        txn_->commit();
        txn_.reset();
    } else {
        SPDLOG_ERROR("Failed to commit transcation. Transcation is closed");
    }
}

std::optional< QueryResult > PsqlTransaction::executeQueryUnsafe(const std::string & query) {
    try {
        pqxx::result result = txn_->exec(query);

        QueryResult queryResult;
        for (const auto & row: result) {
            std::vector< std::variant< int, double, std::string, bool > > rowData;

            for (auto field = 0; field < row.size(); ++field) {
                if (row[field].is_null()) {
                    rowData.push_back(std::string("NULL"));
                } else {
                    rowData.push_back(std::string(row[field].c_str()));
                }
            }

            queryResult.push_back(rowData);
        }

        return queryResult;
    } catch (const std::exception & e) {
        SPDLOG_ERROR("Query execution error: {}", e.what());
        return std::nullopt;
    }
}

std::string PsqlTransaction::escapeString(const std::string & str) {
    return escapeStringStatic(str);
}

std::string PsqlTransaction::escapeStringStatic(const std::string & str) {
    std::string result;
    for (char c: str) {
        if (c == '\'') {
            result += "''";
        } else {
            result += c;
        }
    }
    return result;
}
