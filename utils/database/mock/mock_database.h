#pragma once

#include <utils/database/interface/i_database.h>

#include <gmock/gmock.h>

using namespace testing;

namespace cxx {
    class MockDatabase: public IDatabase {
    public:
        MOCK_METHOD(std::unique_ptr< ITransaction >, makeTransaction, (), (override));
    };

} // namespace cxx
