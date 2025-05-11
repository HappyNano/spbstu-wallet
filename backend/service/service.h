#pragma once

#include <grpcpp/grpcpp.h>

#include <proto/wallet/service.grpc.pb.h>
#include <utils/database/interface/i_database.h>

#include <memory>

namespace wallet {

    /**
     * @class FinanceServiceImpl
     * @brief Implementation of the financial service that provides operations with receipts, transactions, and financial statistics.
     *
     * This service handles all financial operations including:
     * - Receipt scanning and processing
     * - Transactions management (create, update, delete)
     * - Transaction splitting between characters
     * - Characters management
     * - Categories management
     * - Financial statistics and analytics
     */
    class FinanceServiceImpl final: public FinanceService::Service {
    public:
        /**
         * @brief Constructor for FinanceServiceImpl
         * @param db Shared pointer to database interface
         */
        explicit FinanceServiceImpl(std::shared_ptr< cxx::IDatabase > db);

        /**
         * @brief Default destructor
         */
        ~FinanceServiceImpl() override = default;

        /**
         * @brief Authenticates a user by device ID and returns a token
         * @param context The server context
         * @param request The authentication request containing device information
         * @param response The authentication response with token or error information
         * @return Status of the operation
         */
        grpc::Status Authenticate(grpc::ServerContext * context, const AuthRequest * request, AuthResponse * response) override;

        /**
         * @brief Processes a QR code from a receipt and extracts information
         * @param context The server context
         * @param request The QR code request containing either raw code or parsed receipt data
         * @param response The response containing detailed receipt information or error
         * @return Status of the operation
         */
        grpc::Status ProcessQRCode(grpc::ServerContext * context, const QRCodeRequest * request, ReceiptDetailsResponse * response) override;

        /**
         * @brief Retrieves a list of receipts for the authenticated user
         * @param context The server context
         * @param request The request with authentication and filter parameters
         * @param response The response containing a list of receipts or error
         * @return Status of the operation
         */
        grpc::Status GetReceipts(grpc::ServerContext * context, const GetReceiptsRequest * request, ReceiptsResponse * response) override;

        /**
         * @brief Gets detailed information about a specific receipt
         * @param context The server context
         * @param request The request containing receipt ID and authentication
         * @param response The response with detailed receipt information or error
         * @return Status of the operation
         */
        grpc::Status GetReceiptDetails(grpc::ServerContext * context, const GetReceiptDetailsRequest * request, ReceiptDetailsResponse * response) override;

        /**
         * @brief Creates a new financial transaction
         * @param context The server context
         * @param request The request with transaction data
         * @param response The response containing created transaction ID or error
         * @return Status of the operation
         */
        grpc::Status CreateTransaction(grpc::ServerContext * context, const CreateTransactionRequest * request, CreateTransactionResponse * response) override;

        /**
         * @brief Updates an existing transaction
         * @param context The server context
         * @param request The request with updated transaction data
         * @param response The response indicating success or error
         * @return Status of the operation
         */
        grpc::Status UpdateTransaction(grpc::ServerContext * context, const UpdateTransactionRequest * request, Response * response) override;

        /**
         * @brief Deletes a transaction
         * @param context The server context
         * @param request The request containing the transaction ID to delete
         * @param response The response indicating success or error
         * @return Status of the operation
         */
        grpc::Status DeleteTransaction(grpc::ServerContext * context, const DeleteTransactionRequest * request, Response * response) override;

        /**
         * @brief Retrieves a list of transactions with filtering options
         * @param context The server context
         * @param request The request with filter parameters
         * @param response The response with transactions list or error
         * @return Status of the operation
         */
        grpc::Status GetTransactions(grpc::ServerContext * context, const GetTransactionsRequest * request, TransactionsResponse * response) override;

        /**
         * @brief Gets detailed information about a specific transaction
         * @param context The server context
         * @param request The request containing transaction ID
         * @param response The response with transaction details or error
         * @return Status of the operation
         */
        grpc::Status GetTransactionDetails(grpc::ServerContext * context, const GetTransactionDetailsRequest * request, TransactionDetailsResponse * response) override;

        /**
         * @brief Creates a new split of a transaction between characters
         * @param context The server context
         * @param request The request with split details
         * @param response The response containing created split ID or error
         * @return Status of the operation
         */
        grpc::Status CreateSplit(grpc::ServerContext * context, const CreateSplitRequest * request, CreateSplitResponse * response) override;

        /**
         * @brief Updates an existing transaction split
         * @param context The server context
         * @param request The request with updated split data
         * @param response The response indicating success or error
         * @return Status of the operation
         */
        grpc::Status UpdateSplit(grpc::ServerContext * context, const UpdateSplitRequest * request, Response * response) override;

        /**
         * @brief Deletes a transaction split
         * @param context The server context
         * @param request The request containing the split ID to delete
         * @param response The response indicating success or error
         * @return Status of the operation
         */
        grpc::Status DeleteSplit(grpc::ServerContext * context, const DeleteSplitRequest * request, Response * response) override;

        /**
         * @brief Retrieves all characters associated with the authenticated user
         * @param context The server context
         * @param request The request with authentication information
         * @param response The response containing list of characters or error
         * @return Status of the operation
         */
        grpc::Status GetCharacters(grpc::ServerContext * context, const GetCharactersRequest * request, CharactersResponse * response) override;

        /**
         * @brief Creates or updates a character
         * @param context The server context
         * @param request The request with character information
         * @param response The response containing character ID or error
         * @return Status of the operation
         */
        grpc::Status ManageCharacter(grpc::ServerContext * context, const ManageCharacterRequest * request, ManageCharacterResponse * response) override;

        /**
         * @brief Deletes a character
         * @param context The server context
         * @param request The request containing the character ID to delete
         * @param response The response indicating success or error
         * @return Status of the operation
         */
        grpc::Status DeleteCharacter(grpc::ServerContext * context, const ManageCharacterRequest * request, Response * response) override;

        /**
         * @brief Retrieves all expense/income categories
         * @param context The server context
         * @param request The request with authentication information
         * @param response The response containing list of categories or error
         * @return Status of the operation
         */
        grpc::Status GetCategories(grpc::ServerContext * context, const GetCategoriesRequest * request, CategoriesResponse * response) override;

        /**
         * @brief Creates or updates a category
         * @param context The server context
         * @param request The request with category information
         * @param response The response containing category ID or error
         * @return Status of the operation
         */
        grpc::Status ManageCategory(grpc::ServerContext * context, const ManageCategoryRequest * request, ManageCategoryResponse * response) override;

        /**
         * @brief Deletes a category
         * @param context The server context
         * @param request The request containing the category ID to delete
         * @param response The response indicating success or error
         * @return Status of the operation
         */
        grpc::Status DeleteCategory(grpc::ServerContext * context, const ManageCategoryRequest * request, Response * response) override;

        /**
         * @brief Retrieves financial statistics for the authenticated user
         * @param context The server context
         * @param request The request with parameters for statistics calculation
         * @param response The response containing financial statistics or error
         * @return Status of the operation
         */
        grpc::Status GetStatistics(grpc::ServerContext * context, const GetStatisticsRequest * request, StatisticsResponse * response) override;

    private:
        /**
         * @brief Authenticates user by token and retrieves user ID
         * @param token The authentication token
         * @param userId Output parameter to store the authenticated user ID
         * @return True if authentication successful, false otherwise
         */
        bool authenticateUser(const std::string & token, int32_t & userId);

        /**
         * @brief Sets error information in the response
         * @tparam ResponseType The type of response object
         * @param response Pointer to the response object
         * @param code Error code
         * @param message Error message
         * @param details Optional detailed error information
         */
        template < typename ResponseType >
        void setError(ResponseType * response, ErrorInfo::ErrorCode code, const std::string & message, const std::string & details = "");

        /**
         * @brief Converts Google Protobuf Timestamp to receipt date format
         * @param timestamp The Protobuf timestamp
         * @return String representation of timestamp in receipt format (yyyyMMddTHHmmss)
         */
        std::string timestampToReceiptFormat(const google::protobuf::Timestamp & timestamp);

        /**
         * @brief Converts receipt date format to Google Protobuf Timestamp
         * @param receiptDate String representation of date in receipt format
         * @return Protobuf timestamp object
         */
        google::protobuf::Timestamp receiptFormatToTimestamp(const std::string & receiptDate);

        /**
         * @brief Retrieves detailed transaction data from database
         * @param transactionId ID of the transaction
         * @param details Pointer to TransactionDetails object to be filled with data
         * @return True if transaction found and data retrieved, false otherwise
         */
        bool getTransactionData(int32_t transactionId, TransactionDetails * details);

    private:
        /**
         * @brief Database interface for executing queries
         */
        std::shared_ptr< cxx::IDatabase > db_;
    };

} // namespace wallet
