#pragma once

#include <string>

class IReceiptScannerClient {
public:
    struct Response {
        std::string t;
        double s;
        std::string fn;
        std::string i;
        std::string fp;
        int32_t n;
        std::string error{""};
    };

public:
    virtual ~IReceiptScannerClient() = default;

    virtual Response ProcessQRCode(const std::string & user_id, const std::string & qr_code) = 0;
};
