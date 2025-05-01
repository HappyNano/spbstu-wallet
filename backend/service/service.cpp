#include "service.h"

#include <backend/receipt/processor/receipt_processor.h>

#include <spdlog/spdlog.h>

using namespace receipt_scanner;

ReceiptScannerServiceImpl::ReceiptScannerServiceImpl(std::shared_ptr< cxx::IDatabase > db)
  : db_{ std::move(db) } {
}

grpc::Status ReceiptScannerServiceImpl::ProcessQRCode(grpc::ServerContext * /*context*/, const QRCodeRequest * request, ReceiptResponse * response) {
    SPDLOG_INFO("Received request from user: {} with QR code: {}", request->user_id(), request->qr_code_content());

    db_->insert("receipts", { "qrdata" }, { request->qr_code_content() });

    // Обрабатываем QR-код
    auto processedResponse = ReceiptProcessor::processQRCode(request->qr_code_content());

    // Копируем результат в ответ
    if (processedResponse->has_receipt()) {
        response->mutable_receipt()->CopyFrom(processedResponse->receipt());
    } else {
        response->mutable_error()->CopyFrom(processedResponse->error());
    }

    return grpc::Status::OK;
}
