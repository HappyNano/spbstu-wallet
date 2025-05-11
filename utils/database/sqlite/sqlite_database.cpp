#include "sqlite_database.h"
#include "sqlite_transaction.h"

#include <spdlog/spdlog.h>

using namespace cxx;

SQLiteDatabase::~SQLiteDatabase() {
    disconnect();
};

bool SQLiteDatabase::connectInMemory() {
    return connect(":memory:");
}

bool SQLiteDatabase::connect(const std::string & connectionInfo) {
    if (conn_) {
        disconnect();
    }

    int rc = sqlite3_open(connectionInfo.c_str(), &conn_);
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

void SQLiteDatabase::disconnect() {
    if (conn_) {
        sqlite3_close(conn_);
        conn_ = nullptr;
    }
}

std::shared_ptr< ITransaction > SQLiteDatabase::makeTransaction() {
    return std::make_shared< SQLiteTransaction >(conn_);
}

bool SQLiteDatabase::isReady() const noexcept {
    return static_cast< bool >(conn_);
}

std::string SQLiteDatabase::escapeString(const std::string & str) {
    return SQLiteTransaction::escapeStringStatic(str);
}
