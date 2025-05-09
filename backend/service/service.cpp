#include "service.h"

#include <backend/receipt/processor/receipt_processor.h>

#include <spdlog/spdlog.h>

using namespace receipt;

ReceiptScannerServiceImpl::ReceiptScannerServiceImpl(std::shared_ptr< cxx::IDatabase > db)
  : db_{ std::move(db) } {
}

grpc::Status ReceiptScannerServiceImpl::ProcessQRCode(grpc::ServerContext * /*context*/, const QRCodeRequest * request, ReceiptResponse * response) {
    SPDLOG_INFO("Received request from user: {} with QR code: {}", request->user_id(), request->receipt().fn());

    // db_->makeTransaction()->insert("receipts", { "qrdata" }, { request->receipt() });

    // Обрабатываем QR-код
    auto processedResponse = ReceiptProcessor::processQRCode("");

    // Копируем результат в ответ
    if (processedResponse->has_receiptdata()) {
        response->mutable_receiptdata()->mutable_receipt()->CopyFrom(processedResponse->receiptdata().receipt());
    } else {
        response->mutable_error()->CopyFrom(processedResponse->error());
    }

    return grpc::Status::OK;
}
