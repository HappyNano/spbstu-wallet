#pragma once

#include <sqlite3.h>

#include <backend/database/base/base_database.h>

namespace cxx {

    class SQLiteDatabase final: public BaseDatabase {
    public:
        struct InMemory {};

    public:
        SQLiteDatabase() = default;
        ~SQLiteDatabase() override;

        bool connect(const InMemory & connectionInfo);
        bool connect(const std::string & connectionInfo) override;
        void disconnect() override;
        std::optional< QueryResult > executeQuery(const std::string & query) override;

        bool isReady() const noexcept override;

    private:
        bool connectImpl(const std::variant< std::string, InMemory > & connectionInfo);
        std::string escapeString(const std::string & str) override;

    private:
        sqlite3 * conn_ = nullptr;
    };

} // namespace cxx
