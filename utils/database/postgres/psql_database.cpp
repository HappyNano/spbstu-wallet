#include "psql_database.h"
#include "psql_transaction.h"

#include <utils/string/trim.h>

#include <spdlog/spdlog.h>

#include <sstream>

using namespace cxx;

std::string PsqlDatabase::ConnectionInfo::toString() const {
    std::stringstream ss;
    if (dbname.has_value()) {
        ss << "dbname=" << *dbname << ' ';
    }
    if (user.has_value()) {
        ss << "user=" << *user << ' ';
    }
    if (password.has_value()) {
        ss << "password=" << *password << ' ';
    }
    if (host.has_value()) {
        ss << "host=" << *host << ' ';
    }
    if (port.has_value()) {
        ss << "port=" << *port << ' ';
    }
    return rtrimCopy(ss.str());
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
    if (conn_) {
        conn_->close();
    }
    conn_.reset();
}

std::unique_ptr< ITransaction > PsqlDatabase::makeTransaction() {
    return std::make_unique< PsqlTransaction >(std::make_unique< pqxx::work >(*conn_));
}
