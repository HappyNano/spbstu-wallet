#pragma once

#include <sqlite3.h>
#include <utils/database/transaction/base/base_transaction.h>

namespace cxx {

    /**
     * @brief SQLite-specific implementation of BaseTransaction
     *
     * This class provides SQLite-specific functionality for managing database transactions,
     * such as commit, abort, and executing queries against a SQLite database.
     */
    class SQLiteTransaction final: public BaseTransaction {
    public:
        /**
         * @brief Constructor for SQLiteTransaction
         *
         * @param conn Pointer to an active SQLite database connection
         */
        SQLiteTransaction(sqlite3 * conn);

        ~SQLiteTransaction() override;

        // BaseTransaction interface implementation

        /**
         * @brief Aborts (rolls back) the current transaction
         *
         * Cancels all changes made within the current transaction.
         */
        void abort() override;

        /**
         * @brief Commits the current transaction
         *
         * Makes all changes within the current transaction permanent.
         */
        void commit() override;

        /**
         * @brief Checks if a table exists in the SQLite database
         *
         * @param tableName Name of the table to check
         * @return True if the table exists, false otherwise
         */
        bool isTableExist(const std::string & tableName) override;

        static std::string escapeStringStatic(const std::string & str);

    private:
        // BaseTransaction interface implementation

        /**
         * @brief Executes a SQL query directly without additional safety checks
         *
         * @param query The SQL query to execute
         * @return Optional QueryResult containing the result or empty on failure
         */
        std::optional< QueryResult > executeQueryUnsafe(const std::string & query) override;

        /**
         * @brief Escapes a string for safe use in SQLite SQL queries
         *
         * @param str The string to escape
         * @return Escaped string safe for SQLite SQL queries
         */
        std::string escapeString(const std::string & str) override;

    private:
        /**
         * @brief SQLite database connection handle
         *
         * Pointer to the underlying SQLite database connection.
         * Null if no connection is established.
         */
        sqlite3 * conn_ = nullptr;
    };

} // namespace cxx
