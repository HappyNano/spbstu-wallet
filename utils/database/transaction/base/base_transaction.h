#pragma once

#include <utils/database/transaction/interface/i_transaction.h>

namespace cxx {

    /**
     * @brief BaseTransaction class
     * @details You just need to write realization of connect, disconnect, executeQueryUnsafe, isReady methods
     */
    class BaseTransaction: public ITransaction {
    public:
        ~BaseTransaction() override = default;

        // Safe execute query method
        std::optional< QueryResult > executeQuery(const std::string & query) override;

        // Base SQL methods
        bool createTable(const std::string & name, const std::vector< Col > & cols) override;
        bool dropTable(const std::string & tableName) override;
        std::optional< QueryResult > select(const std::string & fromTableName, const std::vector< std::string > & colsName) override;
        bool insert(const std::string & tableName, const std::vector< std::string > & colNames, const std::vector< std::string > & values) override;
        bool update(const std::string & tableName, const std::vector< std::pair< std::string, std::string > > & colValuePairs, const std::string & whereCondition) override;
        bool deleteFrom(const std::string & tableName, const std::string & whereCondition) override;

        // Helpers
        bool isTableExist(const std::string & tableName) override;

    protected:
        virtual std::optional< QueryResult > executeQueryUnsafe(const std::string & query) = 0;
        virtual std::string escapeString(const std::string & str) = 0;

    private:
        auto callWithCheck(auto fn, auto returnFailVal);
    };

} // namespace cxx
