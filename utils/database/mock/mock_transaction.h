#pragma once

#include <utils/database/transaction/interface/i_transaction.h>

#include <gmock/gmock.h>

using namespace testing;

namespace cxx {
    class MockTransaction: public ITransaction {
    public:
        MOCK_METHOD(void, abort, (), (override));
        MOCK_METHOD(void, commit, (), (override));
        MOCK_METHOD(bool, createTable, (const std::string &, const std::vector< Col > &), (override));
        MOCK_METHOD(bool, dropTable, (const std::string &), (override));
        MOCK_METHOD(std::optional< QueryResult >, select, (const std::string &, const std::vector< std::string > &), (override));
        MOCK_METHOD(bool, insert, (const std::string &, const std::vector< std::string > &, const std::vector< std::string > &), (override));
        MOCK_METHOD(bool, update, (const std::string &, (const std::vector< std::pair< std::string, std::string > > &), const std::string &), (override));
        MOCK_METHOD(bool, deleteFrom, (const std::string &, const std::string &), (override));
        MOCK_METHOD(bool, isTableExist, (const std::string &), (override));
        MOCK_METHOD(std::optional< QueryResult >, executeQuery, (const std::string &), (override));
        MOCK_METHOD(std::string, escapeString, (const std::string &), (override));
    };

} // namespace cxx
