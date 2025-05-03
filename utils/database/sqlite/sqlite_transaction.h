#pragma once

#include <sqlite3.h>

#include <utils/database/transaction/base/base_transaction.h>

namespace cxx {

    class SQLiteTransaction final: public BaseTransaction {
    public:
        SQLiteTransaction(sqlite3 * conn);
        ~SQLiteTransaction() override;

        // BaseTransaction
        void abort() override;
        void commit() override;
        bool isTableExist(const std::string & tableName) override;

    private:
        // BaseTransaction
        std::optional< QueryResult > executeQueryUnsafe(const std::string & query) override;
        std::string escapeString(const std::string & str) override;

    private:
        sqlite3 * conn_ = nullptr;
    };

} // namespace cxx
