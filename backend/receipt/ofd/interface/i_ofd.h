#pragma once

#include <backend/receipt/data/qr/qr.h>

namespace wallet {

    class OFDInterface {
    public:
        virtual ~OFDInterface();

        virtual auto getReceiptData(const Receipt & receipt) -> ReceiptData = 0;
    };

} // namespace wallet
