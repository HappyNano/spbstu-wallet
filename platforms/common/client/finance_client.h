#pragma once

#include <grpcpp/grpcpp.h>
#include <proto/wallet/service.grpc.pb.h>

#include <memory>
#include <string>

namespace cxx {

    class FinanceClient {
    public:
        explicit FinanceClient(std::shared_ptr< grpc::Channel > channel);
        ~FinanceClient() = default;

        wallet::AuthResponse Authenticate(const std::string & device_id, const std::string & device_name = "");

        wallet::ReceiptDetailsResponse ProcessQRCode(
         const std::string & token,
         const std::string & qr_code_content,
         const google::protobuf::Timestamp & scanned_at = google::protobuf::Timestamp(),
         const std::string & comment = "",
         int32_t category_id = 0,
         bool create_transaction = false);

        wallet::ReceiptsResponse GetReceipts(
         const std::string & token,
         const google::protobuf::Timestamp & from_date = google::protobuf::Timestamp(),
         const google::protobuf::Timestamp & to_date = google::protobuf::Timestamp(),
         int32_t limit = 0,
         int32_t offset = 0);

        wallet::ReceiptDetailsResponse GetReceiptDetails(const std::string & token, int32_t receipt_id);

        wallet::CreateTransactionResponse CreateTransaction(
         const std::string & token,
         const wallet::TransactionData & transaction);

        wallet::Response UpdateTransaction(
         const std::string & token,
         const wallet::TransactionData & transaction);

        wallet::Response DeleteTransaction(const std::string & token, int32_t transaction_id);

        wallet::TransactionsResponse GetTransactions(
         const std::string & token,
         const google::protobuf::Timestamp & from_date = google::protobuf::Timestamp(),
         const google::protobuf::Timestamp & to_date = google::protobuf::Timestamp(),
         int32_t type = -1,
         int32_t category_id = 0,
         int32_t limit = 0,
         int32_t offset = 0);

        wallet::TransactionDetailsResponse GetTransactionDetails(const std::string & token, int32_t transaction_id);

        wallet::CreateSplitResponse CreateSplit(
         const std::string & token,
         const wallet::TransactionSplitData & split);

        wallet::Response UpdateSplit(
         const std::string & token,
         const wallet::TransactionSplitData & split);

        wallet::Response DeleteSplit(const std::string & token, int32_t split_id);

        wallet::CharactersResponse GetCharacters(const std::string & token);

        wallet::ManageCharacterResponse CreateCharacter(
         const std::string & token,
         const std::string & name);

        wallet::ManageCharacterResponse UpdateCharacter(
         const std::string & token,
         int32_t character_id,
         const std::string & name);

        wallet::Response DeleteCharacter(
         const std::string & token,
         int32_t character_id);

        wallet::CategoriesResponse GetCategories(const std::string & token);

        wallet::ManageCategoryResponse CreateCategory(
         const std::string & token,
         const std::string & name);

        wallet::ManageCategoryResponse UpdateCategory(
         const std::string & token,
         int32_t category_id,
         const std::string & name);

        wallet::Response DeleteCategory(
         const std::string & token,
         int32_t category_id);

        wallet::StatisticsResponse GetStatistics(
         const std::string & token,
         const google::protobuf::Timestamp & from_date = google::protobuf::Timestamp(),
         const google::protobuf::Timestamp & to_date = google::protobuf::Timestamp());

    private:
        std::unique_ptr< wallet::FinanceService::Stub > stub_;

        static wallet::AuthInfo CreateAuthInfo(const std::string & token);
        static google::protobuf::Timestamp CurrentTimestamp();
    };

} // namespace cxx
