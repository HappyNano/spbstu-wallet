#include "finance_client.h"
#include <chrono>

namespace cxx {
    using wallet::Response;

    FinanceClient::FinanceClient(std::shared_ptr< grpc::Channel > channel)
      : stub_(wallet::gateway::FinanceService::NewStub(channel)) {
    }

    wallet::AuthInfo FinanceClient::CreateAuthInfo(const std::string & token) {
        wallet::AuthInfo auth;
        auth.set_token(token);
        return auth;
    }

    google::protobuf::Timestamp FinanceClient::CurrentTimestamp() {
        google::protobuf::Timestamp timestamp;
        auto now = std::chrono::system_clock::now();
        auto seconds = std::chrono::duration_cast< std::chrono::seconds >(now.time_since_epoch()).count();
        auto nanos = std::chrono::duration_cast< std::chrono::nanoseconds >(now.time_since_epoch()).count() % 1000000000;
        timestamp.set_seconds(seconds);
        timestamp.set_nanos(static_cast< int32_t >(nanos));
        return timestamp;
    }

    wallet::AuthResponse FinanceClient::Authenticate(const std::string & device_id, const std::string & device_name) {
        wallet::AuthRequest request;
        request.set_device_id(device_id);
        if (!device_name.empty()) {
            request.set_device_name(device_name);
        }

        wallet::AuthResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->Authenticate(&context, request, &response);
        if (!status.ok()) {
            wallet::ErrorInfo error;
            error.set_code(wallet::ErrorInfo_ErrorCode_SERVER_ERROR);
            error.set_message(status.error_message());
            response.set_allocated_error(new wallet::ErrorInfo(error));
        }

        return response;
    }

    wallet::ReceiptDetailsResponse FinanceClient::ProcessQRCode(
     const std::string & token,
     const std::string & qr_code_content,
     const google::protobuf::Timestamp & scanned_at,
     const std::string & comment,
     int32_t category_id,
     bool create_transaction) {

        wallet::QRCodeRequest request;
        request.set_allocated_auth(new wallet::AuthInfo(CreateAuthInfo(token)));
        request.set_qr_code_content(qr_code_content);

        if (scanned_at.seconds() == 0) {
            request.set_allocated_scanned_at(new google::protobuf::Timestamp(CurrentTimestamp()));
        } else {
            request.set_allocated_scanned_at(new google::protobuf::Timestamp(scanned_at));
        }

        if (!comment.empty()) {
            request.set_comment(comment);
        }

        if (category_id > 0) {
            request.set_category_id(category_id);
        }

        request.set_create_transaction(create_transaction);

        wallet::ReceiptDetailsResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->ProcessQRCode(&context, request, &response);
        if (!status.ok()) {
            wallet::ErrorInfo error;
            error.set_code(wallet::ErrorInfo_ErrorCode_SERVER_ERROR);
            error.set_message(status.error_message());
            response.set_allocated_error(new wallet::ErrorInfo(error));
        }

        return response;
    }

    wallet::ReceiptsResponse FinanceClient::GetReceipts(
     const std::string & token,
     const google::protobuf::Timestamp & from_date,
     const google::protobuf::Timestamp & to_date,
     int32_t limit,
     int32_t offset) {

        wallet::GetReceiptsRequest request;
        request.set_allocated_auth(new wallet::AuthInfo(CreateAuthInfo(token)));

        if (from_date.seconds() > 0) {
            request.set_allocated_from_date(new google::protobuf::Timestamp(from_date));
        }

        if (to_date.seconds() > 0) {
            request.set_allocated_to_date(new google::protobuf::Timestamp(to_date));
        }

        if (limit > 0) {
            request.set_limit(limit);
        }

        if (offset > 0) {
            request.set_offset(offset);
        }

        wallet::ReceiptsResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->GetReceipts(&context, request, &response);
        if (!status.ok()) {
            wallet::ErrorInfo error;
            error.set_code(wallet::ErrorInfo_ErrorCode_SERVER_ERROR);
            error.set_message(status.error_message());
            response.set_allocated_error(new wallet::ErrorInfo(error));
        }

        return response;
    }

    wallet::ReceiptDetailsResponse FinanceClient::GetReceiptDetails(const std::string & token, int32_t receipt_id) {
        wallet::GetReceiptDetailsRequest request;
        request.set_allocated_auth(new wallet::AuthInfo(CreateAuthInfo(token)));
        request.set_receipt_id(receipt_id);

        wallet::ReceiptDetailsResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->GetReceiptDetails(&context, request, &response);
        if (!status.ok()) {
            wallet::ErrorInfo error;
            error.set_code(wallet::ErrorInfo_ErrorCode_SERVER_ERROR);
            error.set_message(status.error_message());
            response.set_allocated_error(new wallet::ErrorInfo(error));
        }

        return response;
    }

    // Implement the remaining methods in a similar fashion...
    wallet::CreateTransactionResponse FinanceClient::CreateTransaction(
     const std::string & token,
     const wallet::TransactionData & transaction) {

        wallet::CreateTransactionRequest request;
        request.set_allocated_auth(new wallet::AuthInfo(CreateAuthInfo(token)));
        request.set_allocated_transaction(new wallet::TransactionData(transaction));

        wallet::CreateTransactionResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->CreateTransaction(&context, request, &response);
        if (!status.ok()) {
            wallet::ErrorInfo error;
            error.set_code(wallet::ErrorInfo_ErrorCode_SERVER_ERROR);
            error.set_message(status.error_message());
            response.set_allocated_error(new wallet::ErrorInfo(error));
        }

        return response;
    }


    Response FinanceClient::UpdateTransaction(
     const std::string & token,
     const wallet::TransactionData & transaction) {

        wallet::UpdateTransactionRequest request;
        request.set_allocated_auth(new wallet::AuthInfo(CreateAuthInfo(token)));
        request.set_allocated_transaction(new wallet::TransactionData(transaction));

        Response response;
        grpc::ClientContext context;

        grpc::Status status = stub_->UpdateTransaction(&context, request, &response);
        if (!status.ok()) {
            wallet::ErrorInfo error;
            error.set_code(wallet::ErrorInfo_ErrorCode_SERVER_ERROR);
            error.set_message(status.error_message());
            response.set_allocated_error(new wallet::ErrorInfo(error));
        }

        return response;
    }

    wallet::category::CategoriesResponse FinanceClient::GetCategories(const std::string & token) {
        wallet::category::GetCategoriesRequest request;
        request.set_allocated_auth(new wallet::AuthInfo(CreateAuthInfo(token)));

        wallet::category::CategoriesResponse response;
        grpc::ClientContext context;

        grpc::Status status = stub_->GetCategories(&context, request, &response);
        if (!status.ok()) {
            wallet::ErrorInfo error;
            error.set_code(wallet::ErrorInfo_ErrorCode_SERVER_ERROR);
            error.set_message(status.error_message());
            response.set_allocated_error(new wallet::ErrorInfo(error));
        }

        return response;
    }

    // Other methods would be implemented similarly

} // namespace cxx
