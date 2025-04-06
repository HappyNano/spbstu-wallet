#include <iostream>
#include <memory>
#include <regex>
#include <string>

#include <grpcpp/grpcpp.h>
#include <proto/wallet/service.grpc.pb.h>

#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include <backend/database/postgres/psql_database.h>

#include <backend/database/postgres/psql_database.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using receipt_scanner::QRCodeRequest;
using receipt_scanner::ReceiptItem;
using receipt_scanner::ReceiptResponse;
using receipt_scanner::ReceiptScannerService;

struct RetailerInfo {
    std::string name;
    std::string address;
    std::vector< ReceiptItem > sample_items;
};

class ReceiptProcessor {
public:
    // Метод для разбора QR-кода чека
    static std::unique_ptr< ReceiptResponse > ProcessQRCode(const std::string & qrCode) {
        auto response = std::make_unique< ReceiptResponse >();

        // Проверяем, что QR-код не пустой
        if (qrCode.empty()) {
            auto * error = response->mutable_error();
            error->set_code(receipt_scanner::ErrorInfo_ErrorCode_PARSING_ERROR);
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
};

// Реализация сервиса для сканирования чеков
class ReceiptScannerServiceImpl final: public ReceiptScannerService::Service {
public:
    ReceiptScannerServiceImpl() {
        db_ = std::make_unique< cxx::PsqlDatabase >();
        db_->connect("dbname=wallet user=admin password=adminadmin host=10.129.0.5 port=5432");
    }

    Status ProcessQRCode(ServerContext * /*context*/, const QRCodeRequest * request, ReceiptResponse * response) override {
        SPDLOG_INFO("Received request from user: {} with QR code: {}", request->user_id(), request->qr_code_content());

        db_->insert("receipts", { "qrdata" }, { request->qr_code_content() });

        // Обрабатываем QR-код
        auto processedResponse = ReceiptProcessor::ProcessQRCode(request->qr_code_content());

        // Копируем результат в ответ
        if (processedResponse->has_receipt()) {
            response->mutable_receipt()->CopyFrom(processedResponse->receipt());
        } else {
            response->mutable_error()->CopyFrom(processedResponse->error());
        }

        return Status::OK;
    }

private:
    std::unique_ptr< cxx::IDatabase > db_;
};

void runServer() {
    std::string serverAddress("0.0.0.0:50051");
    ReceiptScannerServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr< Server > server(builder.BuildAndStart());
    std::cout << "Server listening on " << serverAddress << std::endl;

    server->Wait();
}

int main(int, char **) {
    SPDLOG_INFO("Here1");
    // Init logger
    auto logger = spdlog::stdout_logger_mt("sdlmain");
#ifndef NDEBUG
    logger->set_level(spdlog::level::debug);
#else
    logger->set_level(spdlog::level::info);
#endif
    spdlog::set_default_logger(std::move(logger));
    SPDLOG_INFO("Here2");

    // Отключаем xDS клиент
    setenv("GRPC_XDS_BOOTSTRAP", "{}", 1);
    SPDLOG_INFO("Here3");
    runServer();
    return 0;
}
