#pragma once

#include <grpcpp/grpcpp.h>

#include <backend/database/interface/i_database.h>
#include <proto/wallet/service.grpc.pb.h>

#include <memory>

namespace receipt_scanner {

    class ReceiptScannerServiceImpl final: public ReceiptScannerService::Service {
    public:
        ReceiptScannerServiceImpl(std::unique_ptr< cxx::IDatabase > db);

        grpc::Status ProcessQRCode(grpc::ServerContext * /*context*/, const QRCodeRequest * request, ReceiptResponse * response) override;

    private:
        std::unique_ptr< cxx::IDatabase > db_;
    };

} // namespace receipt_scanner
