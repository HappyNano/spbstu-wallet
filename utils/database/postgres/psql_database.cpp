#include "psql_database.h"

#include <spdlog/spdlog.h>
#include <sstream>

using namespace cxx;

std::string PsqlDatabase::ConnectionInfo::toString() const {
    std::stringstream ss;
    ss << "dbname=" << dbname << ' ';
    ss << "user=" << user << ' ';
    ss << "password=" << password << ' ';
    ss << "host=" << host << ' ';
    ss << "port=" << port;
    return ss.str();
}

PsqlDatabase::PsqlDatabase() = default;

PsqlDatabase::~PsqlDatabase() {
    disconnect();
}

bool PsqlDatabase::connect(const ConnectionInfo & connectionInfo) {
    return connect(connectionInfo.toString());
}

bool PsqlDatabase::connect(const std::string & connectionString) {
    try {
        conn_ = std::make_unique< pqxx::connection >(connectionString);
        return conn_->is_open();
    } catch (const std::exception & e) {
        SPDLOG_ERROR("Connection error: {}", e.what());
        return false;
    }
}

void PsqlDatabase::disconnect() {
    if (isReady()) {
        conn_->close();
    }
    conn_.reset();
}

std::optional< QueryResult > PsqlDatabase::executeQueryUnsafe(const std::string & query) {
    try {
        pqxx::work txn(*conn_);
        pqxx::result result = txn.exec(query);
        txn.commit();

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

bool PsqlDatabase::isReady() const noexcept {
    return conn_ && conn_->is_open();
}

std::string PsqlDatabase::escapeString(const std::string & str) {
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
