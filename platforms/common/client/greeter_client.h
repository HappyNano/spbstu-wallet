#pragma once

#include <memory>
#include <string>
#include <vector>

#include <platforms/common/client/interface/i_greeter_client.h>
#include <grpcpp/grpcpp.h>
#include <proto/wallet/service.grpc.pb.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using wallet::FinanceService;
using wallet::QRCodeRequest;
using wallet::ReceiptResponse;
using wallet::ReceiptData;
using wallet::ReceiptItem;
using wallet::ErrorInfo;

class ReceiptScannerClient: public IReceiptScannerClient {
public:
    ReceiptScannerClient(const std::shared_ptr< Channel > & channel);
    ~ReceiptScannerClient() override = default;

    Response ProcessQRCode(const std::string& user_id, const std::string& qr_code) override;

private:
    std::unique_ptr< FinanceService::Stub > stub_;
};
