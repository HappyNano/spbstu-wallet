#pragma once

#include <backend/receipt/data/qr/qr.h>

namespace receipt {

    class OFDInterface {
    public:
        virtual ~OFDInterface();

        virtual auto getReceiptData(const Receipt& receipt) -> ReceiptData = 0;
    };

} // namespace receipt
