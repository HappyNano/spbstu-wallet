#pragma once

#include <utils/database/transaction/interface/i_transaction.h>

namespace cxx {

    /**
     * @brief Base implementation of ITransaction interface
     *
     * @details This class provides common implementation for database transaction operations.
     * You just need to write realization of connect, disconnect, executeQueryUnsafe, isReady methods
     * in derived classes to support specific database engines.
     */
    class BaseTransaction: public ITransaction {
    public:
        ~BaseTransaction() override = default;

        /**
         * @brief Safely executes a SQL query with error handling
         *
         * @param query The SQL query to execute
         * @return Optional QueryResult containing the result or empty on failure
         */
        std::optional< QueryResult > executeQuery(const std::string & query) override;

        /**
         * @brief Creates a new table in the database
         *
         * @param name Name of the table to create
         * @param cols Vector of columns with their specifications
         * @return True if the table was created successfully, false otherwise
         */
        bool createTable(const std::string & name, const std::vector< Col > & cols) override;

        /**
         * @brief Drops (deletes) a table from the database
         *
         * @param tableName Name of the table to drop
         * @return True if the table was dropped successfully, false otherwise
         */
        bool dropTable(const std::string & tableName) override;

        /**
         * @brief Performs a SELECT query on a table
         *
         * @param fromTableName Table to select from
         * @param colsName Column names to select
         * @return Optional QueryResult containing the selected data or empty on failure
         */
        std::optional< QueryResult > select(const std::string & fromTableName, const std::vector< std::string > & colsName) override;

        /**
         * @brief Inserts data into a table
         *
         * @param tableName Table to insert into
         * @param colNames Names of columns for insertion
         * @param values Values to insert
         * @return True if the data was inserted successfully, false otherwise
         */
        bool insert(const std::string & tableName, const std::vector< std::string > & colNames, const std::vector< std::string > & values) override;

        /**
         * @brief Updates data in a table
         *
         * @param tableName Table to update
         * @param colValuePairs Pairs of column names and their new values
         * @param whereCondition WHERE clause for the update statement
         * @return True if the data was updated successfully, false otherwise
         */
        bool update(const std::string & tableName, const std::vector< std::pair< std::string, std::string > > & colValuePairs, const std::string & whereCondition) override;

        /**
         * @brief Deletes rows from a table
         *
         * @param tableName Table to delete from
         * @param whereCondition WHERE clause for the delete statement
         * @return True if the rows were deleted successfully, false otherwise
         */
        bool deleteFrom(const std::string & tableName, const std::string & whereCondition) override;

        /**
         * @brief Checks if a table exists in the database
         *
         * @param tableName Name of the table to check
         * @return True if the table exists, false otherwise
         */
        bool isTableExist(const std::string & tableName) override;

    protected:
        /**
         * @brief Executes a SQL query without additional safety checks
         *
         * @param query The SQL query to execute
         * @return Optional QueryResult containing the result or empty on failure
         */
        virtual std::optional< QueryResult > executeQueryUnsafe(const std::string & query) = 0;
    };

} // namespace cxx
