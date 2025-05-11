#pragma once

#include "gmock/gmock.h"
#include <utils/database/interface/i_database.h>

#include <gmock/gmock.h>

using namespace testing;

namespace cxx {
    class MockDatabase: public IDatabase {
    public:
        MOCK_METHOD(std::shared_ptr< ITransaction >, makeTransaction, (), (override));
        MOCK_METHOD(bool, isReady, (), (const, noexcept, override));
        MOCK_METHOD(std::string, escapeString, (const std::string &), (override));
    };

} // namespace cxx
