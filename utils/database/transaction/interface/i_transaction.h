#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace cxx {

    struct Col {
        std::string name;
        enum class EDataType {
            INTEGER,
            REAL,
            TEXT,
            BOOLEAN,
            DATE,
            TIMESTAMP
        } type;

        enum class EConstraint {
            NONE,
            PRIMARY_KEY,
            UNIQUE,
            NOT_NULL,
            FOREIGN_KEY
        } constraint = EConstraint::NONE;
    };

    std::string_view dataTypeToSql(Col::EDataType type);
    std::string_view constraintToSql(Col::EConstraint constraint);

    using QueryResult = std::vector< std::vector< std::variant< int, double, std::string, bool > > >;

    /**
     * @brief Interface for Database Transaction
     */
    class ITransaction {
    public:
        virtual ~ITransaction() = default;

        virtual void abort() = 0;
        virtual void commit() = 0;

        // Safe execute query method
        virtual std::optional< QueryResult > executeQuery(const std::string & query) = 0;

        // Base SQL methods
        virtual bool createTable(const std::string & name, const std::vector< Col > & cols) = 0;
        virtual bool dropTable(const std::string & tableName) = 0;
        virtual std::optional< QueryResult > select(const std::string & fromTableName, const std::vector< std::string > & colsName) = 0;
        virtual bool insert(const std::string & tableName, const std::vector< std::string > & colNames, const std::vector< std::string > & values) = 0;
        virtual bool update(const std::string & tableName, const std::vector< std::pair< std::string, std::string > > & colValuePairs, const std::string & whereCondition) = 0;
        virtual bool deleteFrom(const std::string & tableName, const std::string & whereCondition) = 0;

        // Helpers
        virtual bool isTableExist(const std::string & tableName) = 0;
    };

} // namespace cxx
