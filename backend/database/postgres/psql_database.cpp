#include "psql_database.h"

#include <spdlog/spdlog.h>
#include <sstream>

using namespace cxx;

PsqlDatabase::PsqlDatabase()
  : isConnected_(false) {
}

PsqlDatabase::~PsqlDatabase() {
    disconnect();
}

bool PsqlDatabase::connect(const std::string & connection_string) {
    try {
        conn_ = std::make_unique< pqxx::connection >(connection_string);
        isConnected_ = conn_->is_open();
        return isConnected_;
    } catch (const std::exception & e) {
        SPDLOG_ERROR("Connection error: {}", e.what());
        return false;
    }
}

void PsqlDatabase::disconnect() {
    if (conn_ && isConnected_) {
        conn_->close();
        isConnected_ = false;
    }
}

bool PsqlDatabase::createTable(const std::string & name, const std::vector< Col > & cols) {
    if (!isConnected_ || !conn_) {
        return false;
    }

    std::stringstream query;
    query << "CREATE TABLE " << escapeString(name) << " (";

    for (size_t i = 0; i < cols.size(); ++i) {
        const auto & col = cols[i];
        query << escapeString(col.name) << " " << dataTypeToSql(col.type);

        if (col.constraint != Col::EConstraint::NONE) {
            query << " " << constraintToSql(col.constraint);
        }

        if (i < cols.size() - 1) {
            query << ", ";
        }
    }

    query << ");";

    try {
        pqxx::work txn(*conn_);
        txn.exec(query.str());
        txn.commit();
        return true;
    } catch (const std::exception & e) {
        SPDLOG_ERROR("Error creating table: {}", e.what());
        return false;
    }
}

bool PsqlDatabase::dropTable(const std::string & tableName) {
    if (!isConnected_ || !conn_) {
        return false;
    }

    try {
        pqxx::work txn(*conn_);
        txn.exec("DROP TABLE IF EXISTS " + tableName);
        txn.commit();
        return true;
    } catch (const std::exception & e) {
        SPDLOG_ERROR("Error dropping table: {}", e.what());
        return false;
    }
}

std::optional< QueryResult > PsqlDatabase::select(const std::string & fromTableName, const std::vector< std::string > & colsName) {
    if (!isConnected_ || !conn_) {
        return std::nullopt;
    }

    std::stringstream query;
    query << "SELECT ";

    if (colsName.empty()) {
        query << "*";
    } else {
        for (size_t i = 0; i < colsName.size(); ++i) {
            query << escapeString(colsName[i]);
            if (i < colsName.size() - 1) {
                query << ", ";
            }
        }
    }

    query << " FROM " << escapeString(fromTableName) << ";";

    return executeQuery(query.str());
}

bool PsqlDatabase::insert(const std::string & tableName, const std::vector< std::string > & colNames, const std::vector< std::string > & values) {
    if (!isConnected_ || !conn_) {
        return false;
    }
    if (colNames.size() != values.size()) {
        return false;
    }

    std::stringstream query;
    query << "INSERT INTO " << escapeString(tableName) << " (";

    for (size_t i = 0; i < colNames.size(); ++i) {
        query << escapeString(colNames[i]);
        if (i < colNames.size() - 1) {
            query << ", ";
        }
    }

    query << ") VALUES (";

    for (size_t i = 0; i < values.size(); ++i) {
        query << "'" << escapeString(values[i]) << "'";
        if (i < values.size() - 1) {
            query << ", ";
        }
    }

    query << ");";

    try {
        pqxx::work txn(*conn_);
        txn.exec(query.str());
        txn.commit();
        return true;
    } catch (const std::exception & e) {
        SPDLOG_ERROR("Error inserting data: {}", e.what());
        return false;
    }
}

bool PsqlDatabase::update(const std::string & tableName, const std::vector< std::pair< std::string, std::string > > & colValuePairs, const std::string & whereCondition) {
    if (!isConnected_ || !conn_) {
        return false;
    }
    if (colValuePairs.empty()) {
        return false;
    }

    std::stringstream query;
    query << "UPDATE " << escapeString(tableName) << " SET ";

    for (size_t i = 0; i < colValuePairs.size(); ++i) {
        query << escapeString(colValuePairs[i].first) << " = '" << escapeString(colValuePairs[i].second) << "'";
        if (i < colValuePairs.size() - 1) {
            query << ", ";
        }
    }

    if (!whereCondition.empty()) {
        query << " WHERE " << whereCondition;
    }

    query << ";";

    try {
        pqxx::work txn(*conn_);
        txn.exec(query.str());
        txn.commit();
        return true;
    } catch (const std::exception & e) {
        SPDLOG_ERROR("Error updating data: {}", e.what());
        return false;
    }
}

bool PsqlDatabase::deleteFrom(const std::string & tableName, const std::string & whereCondition) {
    if (!isConnected_ || !conn_) {
        return false;
    }

    std::stringstream query;
    query << "DELETE FROM " << escapeString(tableName);

    if (!whereCondition.empty()) {
        query << " WHERE " << whereCondition;
    }

    query << ";";

    try {
        pqxx::work txn(*conn_);
        txn.exec(query.str());
        txn.commit();
        return true;
    } catch (const std::exception & e) {
        SPDLOG_ERROR("Error deleting data: {}", e.what());
        return false;
    }
}

std::optional< QueryResult > PsqlDatabase::executeQuery(const std::string & query) {
    if (!isConnected_ || !conn_) {
        return std::nullopt;
    }

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
                    // Для упрощения, возвращаем все как строки
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
