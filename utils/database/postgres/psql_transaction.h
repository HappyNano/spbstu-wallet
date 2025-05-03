#pragma once

#include <optional>
#include <pqxx/pqxx>

#include <utils/database/transaction/base/base_transaction.h>

namespace cxx {

    class PsqlTransaction final: public BaseTransaction {
    public:
        PsqlTransaction(std::unique_ptr< pqxx::work > txn);
        ~PsqlTransaction() override;

        // BaseTransaction
        void abort() override;
        void commit() override;

    private:
        // BaseTransaction
        std::optional< QueryResult > executeQueryUnsafe(const std::string & query) override;
        std::string escapeString(const std::string & str) override;

    private:
        std::unique_ptr< pqxx::work > txn_;
    };

} // namespace cxx
