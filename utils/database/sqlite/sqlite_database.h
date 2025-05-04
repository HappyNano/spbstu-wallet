#pragma once

#include <sqlite3.h>

#include <utils/database/interface/i_database.h>

namespace cxx {

    class SQLiteDatabase final: public IDatabase {
    public:
        SQLiteDatabase() = default;
        ~SQLiteDatabase() override;

        bool connectInMemory();
        bool connect(const std::string & connectionInfo);
        void disconnect();

        // IDatabase
        std::shared_ptr< ITransaction > makeTransaction() override;
        bool isReady() const noexcept override;

    private:
        sqlite3 * conn_ = nullptr;
    };

} // namespace cxx
