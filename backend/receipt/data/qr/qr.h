#pragma once

#include <proto/wallet/receipt/receipt.pb.h>

#include <string>

namespace wallet {
    bool isValidQRData(const std::string & data);
    Receipt parseQRDataFromString(const std::string & data);
} // namespace wallet
