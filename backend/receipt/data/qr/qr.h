#pragma once

#include <proto/wallet/receipt/receipt.pb.h>

#include <string>

namespace receipt {
    bool isValidQRData(const std::string & data);
    Receipt parseQRDataFromString(const std::string & data);
} // namespace receipt
