#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace cxx {

    /**
     * @brief Represents a database column with its properties
     */
    struct Col {
        /** @brief Name of the column */
        std::string name;

        /**
         * @brief Enumeration of supported data types for columns
         */
        enum class EDataType {
            INTEGER,  /**< Integer data type */
            REAL,     /**< Floating point data type */
            TEXT,     /**< Text/string data type */
            BOOLEAN,  /**< Boolean data type */
            DATE,     /**< Date data type */
            TIMESTAMP /**< Timestamp data type */
        } type;

        /**
         * @brief Enumeration of column constraints
         */
        enum class EConstraint {
            NONE,        /**< No constraint */
            PRIMARY_KEY, /**< Primary key constraint */
            UNIQUE,      /**< Unique constraint */
            NOT_NULL,    /**< Not null constraint */
            FOREIGN_KEY  /**< Foreign key constraint */
        } constraint = EConstraint::NONE;
    };

    /**
     * @brief Converts a data type enum to its SQL representation
     * @param type The data type to convert
     * @return SQL string representation of the data type
     */
    std::string_view dataTypeToSql(Col::EDataType type);

    /**
     * @brief Converts a constraint enum to its SQL representation
     * @param constraint The constraint to convert
     * @return SQL string representation of the constraint
     */
    std::string_view constraintToSql(Col::EConstraint constraint);

    /**
     * @brief Represents the result of a database query
     *
     * A two-dimensional vector where the outer vector represents rows
     * and the inner vector represents columns. Each cell can contain
     * different types of data.
     */
    using QueryResult = std::vector< std::vector< std::variant< int, double, std::string, bool > > >;

    /**
     * @brief Interface for Database Transaction
     */
    class ITransaction {
    public:
        /**
         * @brief Virtual destructor for proper cleanup
         */
        virtual ~ITransaction() = default;

        /**
         * @brief Aborts the current transaction
         */
        virtual void abort() = 0;

        /**
         * @brief Commits the current transaction
         */
        virtual void commit() = 0;

        /**
         * @brief Executes a raw SQL query
         * @param query The SQL query to execute
         * @return Optional result set (empty if query doesn't return data or fails)
         */
        virtual std::optional< QueryResult > executeQuery(const std::string & query) = 0;

        /**
         * @brief Creates a new table in the database
         * @param name Name of the table to create
         * @param cols Vector of columns with their specifications
         * @return True if successful, false otherwise
         */
        virtual bool createTable(const std::string & name, const std::vector< Col > & cols) = 0;

        /**
         * @brief Drops (deletes) a table from the database
         * @param tableName Name of the table to drop
         * @return True if successful, false otherwise
         */
        virtual bool dropTable(const std::string & tableName) = 0;

        /**
         * @brief Performs a SELECT query on a table
         * @param fromTableName Table to select from
         * @param colsName Column names to select
         * @return Optional result set of the query
         */
        virtual std::optional< QueryResult > select(const std::string & fromTableName, const std::vector< std::string > & colsName) = 0;

        /**
         * @brief Inserts data into a table
         * @param tableName Table to insert into
         * @param colNames Names of columns for insertion
         * @param values Values to insert
         * @return True if successful, false otherwise
         */
        virtual bool insert(const std::string & tableName, const std::vector< std::string > & colNames, const std::vector< std::string > & values) = 0;

        /**
         * @brief Updates data in a table
         * @param tableName Table to update
         * @param colValuePairs Pairs of column names and their new values
         * @param whereCondition WHERE clause for the update statement
         * @return True if successful, false otherwise
         */
        virtual bool update(const std::string & tableName, const std::vector< std::pair< std::string, std::string > > & colValuePairs, const std::string & whereCondition) = 0;

        /**
         * @brief Deletes rows from a table
         * @param tableName Table to delete from
         * @param whereCondition WHERE clause for the delete statement
         * @return True if successful, false otherwise
         */
        virtual bool deleteFrom(const std::string & tableName, const std::string & whereCondition) = 0;

        /**
         * @brief Checks if a table exists in the database
         * @param tableName Name of the table to check
         * @return True if the table exists, false otherwise
         */
        virtual bool isTableExist(const std::string & tableName) = 0;
    };

} // namespace cxx
