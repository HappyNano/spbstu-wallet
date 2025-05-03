#pragma once

#include <utils/database/transaction/interface/i_transaction.h>

#include <memory>

namespace cxx {

    class IDatabase {
    public:
        virtual ~IDatabase();

        virtual std::unique_ptr< ITransaction > makeTransaction() = 0;
    };

} // namespace cxx
