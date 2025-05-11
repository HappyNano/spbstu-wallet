#pragma once

#include <optional>
#include <pqxx/pqxx>

#include <utils/database/transaction/base/base_transaction.h>

namespace cxx {

    /**
     * @brief PostgreSQL-specific implementation of BaseTransaction
     *
     * This class provides PostgreSQL-specific functionality for managing database transactions,
     * such as commit, abort, and executing queries against a PostgreSQL database.
     */
    class PsqlTransaction final: public BaseTransaction {
    public:
        /**
         * @brief Constructor for PsqlTransaction
         *
         * @param txn Unique pointer to a pqxx transaction object
         */
        PsqlTransaction(std::unique_ptr< pqxx::work > txn);

        ~PsqlTransaction() override;

        // BaseTransaction interface implementation

        /**
         * @brief Aborts (rolls back) the current transaction
         *
         * Cancels all changes made within the current PostgreSQL transaction.
         */
        void abort() override;

        /**
         * @brief Commits the current transaction
         *
         * Makes all changes within the current PostgreSQL transaction permanent.
         */
        void commit() override;

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
         * @brief Escapes a string for safe use in PostgreSQL SQL queries
         *
         * @param str The string to escape
         * @return Escaped string safe for PostgreSQL SQL queries
         */
        std::string escapeString(const std::string & str) override;

    private:
        /**
         * @brief PostgreSQL transaction object
         *
         * Unique pointer to the underlying pqxx transaction object
         * that manages the actual database transaction.
         */
        std::unique_ptr< pqxx::work > txn_;
    };

} // namespace cxx
