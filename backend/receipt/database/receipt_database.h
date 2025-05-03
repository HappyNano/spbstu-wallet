#pragma once

#include <utils/database/interface/i_database.h>

#include <proto/wallet/service.pb.h>

#include <memory>
#include <string>

namespace receipt_scanner {

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

        void insertReceiptData(const std::string & qrCode);
        void insertReceiptData(const ReceiptData & receipt);

        // auto getReceipt

    private:
        const Config config_;
        const std::shared_ptr< cxx::IDatabase > database_;
    };

} // namespace receipt_scanner
