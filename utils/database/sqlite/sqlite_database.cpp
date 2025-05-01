#include "sqlite_database.h"

#include <spdlog/spdlog.h>

using namespace cxx;

SQLiteDatabase::~SQLiteDatabase() {
    disconnect();
};

bool SQLiteDatabase::connect(const InMemory & connectionInfo) {
    return connectImpl(connectionInfo);
}

bool SQLiteDatabase::connect(const std::string & connectionInfo) {
    return connectImpl(connectionInfo);
}

void SQLiteDatabase::disconnect() {
    if (conn_) {
        sqlite3_close(conn_);
        conn_ = nullptr;
    }
}

std::optional< QueryResult > SQLiteDatabase::executeQueryUnsafe(const std::string & query) {
    sqlite3_stmt * stmt = nullptr;
    int rc = sqlite3_prepare_v2(conn_, query.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        SPDLOG_ERROR("SQLite prepare error: {}", sqlite3_errmsg(conn_));
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
        SPDLOG_ERROR("SQLite step error: {}", sqlite3_errmsg(conn_));
        return std::nullopt;
    }

    return result;
}

bool SQLiteDatabase::isReady() const noexcept {
    return conn_;
}

bool SQLiteDatabase::connectImpl(const std::variant< std::string, InMemory > & connectionInfo) {
    if (conn_) {
        disconnect();
    }

    int rc;
    if (std::holds_alternative< InMemory >(connectionInfo)) {
        // Создаем in-memory базу данных
        rc = sqlite3_open(":memory:", &conn_);
    } else {
        // Подключаемся к файловой базе данных
        const auto & path = std::get< std::string >(connectionInfo);
        rc = sqlite3_open(path.c_str(), &conn_);
    }

    if (rc != SQLITE_OK) {
        if (conn_) {
            SPDLOG_ERROR("SQLite error: {}", sqlite3_errmsg(conn_));
            sqlite3_close(conn_);
            conn_ = nullptr;
        } else {
            SPDLOG_ERROR("SQLite error: Unable to allocate memory for database connection");
        }
        return false;
    }

    return true;
}

std::string SQLiteDatabase::escapeString(const std::string & str) {
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

bool SQLiteDatabase::isTableExist(const std::string & tableName) {
    std::stringstream query;
    query << "SELECT name FROM sqlite_master WHERE type='table' AND name='";
    query << escapeString(tableName);
    query << "';";

    auto result = executeQuery(query.str());
    return result.has_value() && !result->empty();
}
