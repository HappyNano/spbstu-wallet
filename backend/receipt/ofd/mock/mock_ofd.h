#pragma once

#include <backend/receipt/ofd/interface/i_ofd.h>

#include <gmock/gmock.h>

using namespace testing;

namespace receipt {

    class MockOFD final: public OFDInterface {
        MOCK_METHOD(ReceiptData, getReceiptData, (const Receipt&), (override));
    };

} // namespace receipt
