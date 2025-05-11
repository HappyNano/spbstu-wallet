#pragma once

#include <grpcpp/grpcpp.h>

#include <proto/wallet/service.grpc.pb.h>
#include <utils/database/interface/i_database.h>

#include <memory>

namespace wallet {

    class FinanceServiceImpl final: public FinanceService::Service {
    public:
        explicit FinanceServiceImpl(std::shared_ptr< cxx::IDatabase > db);
        ~FinanceServiceImpl() override = default;

        // Авторизация
        grpc::Status Authenticate(grpc::ServerContext * context, const AuthRequest * request, AuthResponse * response) override;

        // Чеки
        grpc::Status ProcessQRCode(grpc::ServerContext * context, const QRCodeRequest * request, ReceiptDetailsResponse * response) override;

        grpc::Status GetReceipts(grpc::ServerContext * context, const GetReceiptsRequest * request, ReceiptsResponse * response) override;

        grpc::Status GetReceiptDetails(grpc::ServerContext * context, const GetReceiptDetailsRequest * request, ReceiptDetailsResponse * response) override;

        // Транзакции
        grpc::Status CreateTransaction(grpc::ServerContext * context, const CreateTransactionRequest * request, CreateTransactionResponse * response) override;

        grpc::Status UpdateTransaction(grpc::ServerContext * context, const UpdateTransactionRequest * request, Response * response) override;

        grpc::Status DeleteTransaction(grpc::ServerContext * context, const DeleteTransactionRequest * request, Response * response) override;

        grpc::Status GetTransactions(grpc::ServerContext * context, const GetTransactionsRequest * request, TransactionsResponse * response) override;

        grpc::Status GetTransactionDetails(grpc::ServerContext * context, const GetTransactionDetailsRequest * request, TransactionDetailsResponse * response) override;

        // Деления транзакций
        grpc::Status CreateSplit(grpc::ServerContext * context, const CreateSplitRequest * request, CreateSplitResponse * response) override;

        grpc::Status UpdateSplit(grpc::ServerContext * context, const UpdateSplitRequest * request, Response * response) override;

        grpc::Status DeleteSplit(grpc::ServerContext * context, const DeleteSplitRequest * request, Response * response) override;

        // Персонажи
        grpc::Status GetCharacters(grpc::ServerContext * context, const GetCharactersRequest * request, CharactersResponse * response) override;

        grpc::Status ManageCharacter(grpc::ServerContext * context, const ManageCharacterRequest * request, ManageCharacterResponse * response) override;

        grpc::Status DeleteCharacter(grpc::ServerContext * context, const ManageCharacterRequest * request, Response * response) override;

        // Категории
        grpc::Status GetCategories(grpc::ServerContext * context, const GetCategoriesRequest * request, CategoriesResponse * response) override;

        grpc::Status ManageCategory(grpc::ServerContext * context, const ManageCategoryRequest * request, ManageCategoryResponse * response) override;

        grpc::Status DeleteCategory(grpc::ServerContext * context, const ManageCategoryRequest * request, Response * response) override;

        // Статистика
        grpc::Status GetStatistics(grpc::ServerContext * context, const GetStatisticsRequest * request, StatisticsResponse * response) override;

    private:
        bool authenticateUser(const std::string & token, int32_t & userId);

        // Заполнение структуры ошибки в ответе
        template < typename ResponseType >
        void setError(ResponseType * response, ErrorInfo::ErrorCode code, const std::string & message, const std::string & details = "");

        // Преобразование времени между различными форматами
        std::string timestampToReceiptFormat(const google::protobuf::Timestamp & timestamp);
        google::protobuf::Timestamp receiptFormatToTimestamp(const std::string & receiptDate);

        // Методы для работы с транзакциями
        bool getTransactionData(int32_t transactionId, TransactionDetails * details);

        std::shared_ptr< cxx::IDatabase > db_;
    };

} // namespace wallet
