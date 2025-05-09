#pragma once

#include <backend/receipt/ofd/interface/i_ofd.h>
#include <utils/http/client/interface/i_http_client.h>

namespace receipt {

    class HttpOFD final: public OFDInterface {
    public:
        struct Settings {
            std::string token;
        };

    public:
        HttpOFD(std::shared_ptr< cxx::IHttpClient > httpClient, Settings settings);
        ~HttpOFD() override = default;

        auto getReceiptData(const Receipt & receipt) -> ReceiptData override;

    private:
        auto makeRequestBody(const Receipt & receipt) const -> std::string;

    private:
        const Settings settings_;
        const std::shared_ptr< cxx::IHttpClient > httpClient_;
    };

} // namespace receipt
