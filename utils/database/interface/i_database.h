#pragma once

#include <utils/database/transaction/interface/i_transaction.h>

#include <memory>

namespace cxx {

    class IDatabase {
    public:
        virtual ~IDatabase();

        virtual std::shared_ptr< ITransaction > makeTransaction() = 0;
        virtual bool isReady() const noexcept = 0;
    };

} // namespace cxx
