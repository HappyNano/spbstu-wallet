#pragma once

#include <proto/wallet/service.pb.h>

#include <memory>
#include <string>

namespace receipt {

    class ReceiptProcessor {
    public:
        static std::unique_ptr< ReceiptResponse > processQRCode(const std::string & qrCode);
    };

} // namespace receipt
