#include "receipt_processor.h"

#include <regex>

using namespace receipt_scanner;

std::unique_ptr< ReceiptResponse > ReceiptProcessor::processQRCode(const std::string & qrCode) {
    auto response = std::make_unique< ReceiptResponse >();

    // Проверяем, что QR-код не пустой
    if (qrCode.empty()) {
        auto * error = response->mutable_error();
        error->set_code(ErrorInfo_ErrorCode_PARSING_ERROR);
        error->set_message("Empty QR code");
        return response;
    }

    // Проверяем общий формат фискального QR-кода
    std::regex fiscalQRRegex(
     "t=\\d{8}T\\d{4}&" // Дата и время (t)
     "s=\\d+\\.?\\d*&"  // Сумма (s)
     "fn=\\d+&"         // Номер фискального накопителя (fn)
     "i=\\d+&"          // Номер фискального документа (i)
     "fp=\\d+&"         // Фискальный признак (fp)
     "n=\\d+"           // Тип документа (n)
    );

    if (!std::regex_match(qrCode, fiscalQRRegex)) {
        // Проверяем, содержит ли QR хотя бы какие-то параметры
        std::regex paramRegex("([^=&]+)=([^=&]+)");
        if (!std::regex_search(qrCode, paramRegex)) {
            auto * error = response->mutable_error();
            error->set_code(receipt_scanner::ErrorInfo_ErrorCode_UNKNOWN_RECEIPT_FORMAT);
            error->set_message("Invalid QR code format");
            error->set_details("QR code doesn't match fiscal receipt pattern: " + qrCode);
            return response;
        }
    }

    // Парсим параметры QR-кода
    std::map< std::string, std::string > params;
    std::regex paramRegex("([^=&]+)=([^=&]+)");
    auto paramBegin = std::sregex_iterator(qrCode.begin(), qrCode.end(), paramRegex);
    auto paramEnd = std::sregex_iterator();

    for (auto i = paramBegin; i != paramEnd; ++i) {
        std::smatch match = *i;
        params[match[1].str()] = match[2].str();
    }

    // Проверяем наличие всех необходимых полей
    const std::vector< std::string > requiredFields = { "t", "s", "fn", "i", "fp", "n" };
    std::vector< std::string > missingFields;

    for (const auto & field: requiredFields) {
        if (params.find(field) == params.end()) {
            missingFields.push_back(field);
        }
    }

    if (!missingFields.empty()) {
        auto * error = response->mutable_error();
        error->set_code(receipt_scanner::ErrorInfo_ErrorCode_PARSING_ERROR);
        error->set_message("Missing required fields in QR code");

        std::string details = "Missing fields: ";
        for (size_t i = 0; i < missingFields.size(); ++i) {
            details += missingFields[i];
            if (i < missingFields.size() - 1) {
                details += ", ";
            }
        }
        error->set_details(details);
        return response;
    }

    // Проверка корректности полей
    try {
        // Проверка суммы
        double amount = std::stod(params["s"]);
        if (amount < 0) {
            throw std::runtime_error("Invalid amount (s): " + params["s"]);
        }

        // Проверка типа документа
        int docType = std::stoi(params["n"]);
        if (docType < 1 || docType > 4) {
            throw std::runtime_error("Invalid document type (n): " + params["n"]);
        }

        // Если все проверки прошли успешно, создаем ответ с данными чека
        auto * receipt = response->mutable_receipt();

        // Заполняем основные поля из QR-кода
        receipt->set_t(params["t"]);
        receipt->set_s(amount);
        receipt->set_fn(params["fn"]);
        receipt->set_i(params["i"]);
        receipt->set_fp(params["fp"]);
        receipt->set_n(docType);

        // Устанавливаем способ оплаты
        // receipt->set_payment_method(ReceiptData_PaymentMethod_PAYMENT_CARD);

        // Добавляем дополнительную информацию
        (*receipt->mutable_additional_info())["processed_at"] = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());

    } catch (const std::exception & e) {
        auto * error = response->mutable_error();
        error->set_code(receipt_scanner::ErrorInfo_ErrorCode_PARSING_ERROR);
        error->set_message("Error parsing QR code fields");
        error->set_details(e.what());
        return response;
    }

    return response;
}
