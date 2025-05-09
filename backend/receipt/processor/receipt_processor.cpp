#include "receipt_processor.h"

#include <backend/receipt/data/qr/qr.h>

using namespace receipt;

std::unique_ptr< ReceiptResponse > ReceiptProcessor::processQRCode(const std::string & qrCode) {
    auto response = std::make_unique< ReceiptResponse >();

    // Проверяем, что QR-код не пустой
    if (qrCode.empty()) {
        auto * error = response->mutable_error();
        error->set_code(ErrorInfo_ErrorCode_PARSING_ERROR);
        error->set_message("Empty QR code");
        return response;
    }

    if (!receipt::isValidQRData(qrCode)) {
        auto * error = response->mutable_error();
        error->set_code(receipt::ErrorInfo_ErrorCode_UNKNOWN_RECEIPT_FORMAT);
        error->set_message("Invalid QR code format");
        error->set_details("QR code doesn't match fiscal receipt pattern: " + qrCode);
        return response;
    }

    // Проверка корректности полей
    try {
        auto receipt = receipt::parseQRDataFromString(qrCode);

        auto * receiptData = response->mutable_receiptdata();
        receiptData->mutable_receipt()->CopyFrom(receipt);

        // Устанавливаем способ оплаты
        // receipt->set_payment_method(ReceiptData_PaymentMethod_PAYMENT_CARD);

        // Добавляем дополнительную информацию
        (*receiptData->mutable_additional_info())["processed_at"] = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());

    } catch (const std::exception & e) {
        auto * error = response->mutable_error();
        error->set_code(receipt::ErrorInfo_ErrorCode_PARSING_ERROR);
        error->set_message("Error parsing QR code fields");
        error->set_details(e.what());
        return response;
    }

    return response;
}
