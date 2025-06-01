#include "sqlite_transaction.h"

#include <spdlog/spdlog.h>

using namespace cxx;

SQLiteTransaction::SQLiteTransaction(sqlite3 * conn)
  : conn_(conn) {
}

SQLiteTransaction::~SQLiteTransaction() {
    SPDLOG_DEBUG("Closing SQLiteTransaction");
}

void SQLiteTransaction::abort() {
}

void SQLiteTransaction::commit() {
}

bool SQLiteTransaction::isTableExist(const std::string & tableName) {
    std::stringstream query;
    query << "SELECT name FROM sqlite_master WHERE type='table' AND name='";
    query << escapeString(tableName);
    query << "';";

    auto result = executeQuery(query.str());
    return result.has_value() && !result->empty();
}

std::optional< QueryResult > SQLiteTransaction::executeQueryUnsafe(const std::string & query) {
    sqlite3_stmt * stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn_, query.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        SPDLOG_ERROR("SQLite prepare error: " + std::string(sqlite3_errmsg(conn_)));
        return std::nullopt;
    }

    QueryResult result;
    int columnCount = sqlite3_column_count(stmt);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::vector< std::variant< int, double, std::string, bool > > row;
        row.reserve(columnCount);

        for (int i = 0; i < columnCount; ++i) {
            int type = sqlite3_column_type(stmt, i);

            switch (type) {
            case SQLITE_INTEGER: {
                int val = sqlite3_column_int(stmt, i);
                // Если это булевское значение (0 или 1)
                if (std::string(sqlite3_column_decltype(stmt, i) ? sqlite3_column_decltype(stmt, i) : "") == "BOOLEAN") {
                    row.push_back(val != 0);
                } else {
                    row.push_back(val);
                }
                break;
            }
            case SQLITE_FLOAT: {
                double val = sqlite3_column_double(stmt, i);
                row.push_back(val);
                break;
            }
            case SQLITE_TEXT: {
                const char * text = reinterpret_cast< const char * >(sqlite3_column_text(stmt, i));
                row.push_back(std::string(text));
                break;
            }
            case SQLITE_NULL: {
                // По умолчанию используем строку для NULL значений
                row.push_back(std::string(""));
                break;
            }
            default: {
                // Для неизвестных типов также используем строку
                row.push_back(std::string(""));
                break;
            }
            }
        }
        result.push_back(std::move(row));
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        SPDLOG_ERROR("SQLite step error: " + std::string(sqlite3_errmsg(conn_)));
        return std::nullopt;
    }

    return result;
}

std::string SQLiteTransaction::escapeString(const std::string & str) {
    return escapeStringStatic(str);
}

std::string SQLiteTransaction::escapeStringStatic(const std::string & str) {
    std::string result;
    result.reserve(str.size() * 2);

    for (char c: str) {
        if (c == '\'') {
            result += "''";
        } else {
            result += c;
        }
    }

    return result;
}
