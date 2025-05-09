#pragma once

#include <utils/database/interface/i_database.h>
#include <proto/wallet/receipt/receipt.pb.h>

#include <memory>
#include <string>

namespace receipt {

    class ReceiptDatabase final {
    public:
        struct Config {
            std::string tableName;

            bool isValid() const noexcept;
        };

    public:
        using this_t = ReceiptDatabase;

        ReceiptDatabase(Config config, std::shared_ptr< cxx::IDatabase > database);
        ReceiptDatabase(const this_t &) = delete;

        void init();

        void insertReceipt(const std::string & token, const Receipt & receipt);
        void insertReceiptData(const std::string & qrCode);
        void insertReceiptData(const std::string & token, const ReceiptData & receipt);

        // auto getReceipt

    private:
        const Config config_;
        const std::shared_ptr< cxx::IDatabase > database_;
    };

} // namespace receipt
