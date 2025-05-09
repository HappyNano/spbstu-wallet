#pragma once

#include <backend/receipt/ofd/interface/i_ofd.h>

namespace receipt {

    class NullOFD: public OFDInterface {
    public:
        ~NullOFD() override = default;

        auto getReceiptData(const Receipt& receipt) -> ReceiptData override;
    };

} // namespace receipt
