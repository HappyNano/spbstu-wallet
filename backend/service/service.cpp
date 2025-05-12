#include "service.h"

#include <google/protobuf/util/time_util.h>
#include <grpcpp/impl/codegen/status.h>

#include <backend/receipt/data/qr/qr.h>

#include <iomanip>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace wallet;
using google::protobuf::Timestamp;
using google::protobuf::util::TimeUtil;

namespace {

    Timestamp stringToProtoTimestamp(const std::string & timestampStr) {
        Timestamp protoTimestamp;

        struct tm tm = {};
        char buffer[100];
        strncpy(buffer, timestampStr.c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';

        char * pos = strchr(buffer, '.');
        if (pos != nullptr) {
            *pos = '\0';
        }

        if (strptime(buffer, "%Y-%m-%d %H:%M:%S", &tm) != nullptr || strptime(buffer, "%Y-%m-%dT%H:%M:%S", &tm) != nullptr) {

            time_t time = mktime(&tm);

            protoTimestamp.set_seconds(time);
            protoTimestamp.set_nanos(0);
        } else {

            time_t now = time(nullptr);
            protoTimestamp.set_seconds(now);
            protoTimestamp.set_nanos(0);
        }

        return protoTimestamp;
    }

    template < class... Ts >
    struct Overloaded: Ts... {
        using Ts::operator()...;
    };
    template < class... Ts >
    Overloaded(Ts...) -> Overloaded< Ts... >;

    template < typename T >
    T getVariantValue(const std::variant< int, double, std::string, bool > & value) {
        return std::visit(
         Overloaded{
          [](int v) -> T { if constexpr (std::is_same_v<std::string, T>) {return std::to_string(v);} else {return static_cast< T >(v);} },
          [](double v) -> T { if constexpr (std::is_same_v<std::string, T>) {return std::to_string(v);} else {return static_cast< T >(v);} },
          [](const std::string & v) -> T {
              if constexpr (std::is_same_v< T, std::string >) {
                  return v;
              } else {
                  try {
                      if constexpr (std::is_same_v< T, int >) {
                          return std::stoi(v);
                      } else if constexpr (std::is_same_v< T, int32_t >) {
                          return std::stoi(v);
                      } else if constexpr (std::is_same_v< T, int64_t >) {
                          return std::stoll(v);
                      } else if constexpr (std::is_same_v< T, double >) {
                          return std::stod(v);
                      } else if constexpr (std::is_same_v< T, bool >) {
                          return v == "true" || v == "1" || v == "t";
                      }
                  } catch (...) {
                      return T{};
                  }
              }
              throw std::runtime_error("Bad variant");
          },
          [](bool v) -> T {
              if constexpr (std::is_same_v< std::string, T >) {
                  return std::to_string(v);
              } else {
                  return static_cast< T >(v);
              }
          } },
         value);
    }

}

FinanceServiceImpl::FinanceServiceImpl(std::shared_ptr< cxx::IDatabase > db)
  : db_(std::move(db)) {
}

bool FinanceServiceImpl::authenticateUser(const std::string & token, int32_t & userId) {
    if (token.empty()) {
        return false;
    }

    try {
        std::string query = "SELECT id FROM users WHERE token = '" + db_->escapeString(token) + "'";

        auto resultOpt = db_->makeTransaction()->executeQuery(query);

        if (!resultOpt.has_value() || resultOpt.value().empty()) {
            return false;
        }

        userId = getVariantValue< int32_t >(resultOpt.value()[0][0]);
        return true;
    } catch (const std::exception & e) {

        return false;
    }
}

grpc::Status FinanceServiceImpl::Authenticate(grpc::ServerContext * /*context*/, const AuthRequest * request, AuthResponse * response) {
    try {
        std::string deviceId = request->device_id();
        std::string deviceName = request->has_device_name() ? request->device_name() : "Unknown Device";

        if (deviceId.empty()) {
            setError(response, ErrorInfo::INVALID_REQUEST, "Device ID is required");
            return grpc::Status::OK;
        }

        std::string token;

        std::string query = "SELECT u.token FROM users u JOIN users_data ud ON u.id = ud.user_id "
                            "WHERE ud.device_id = '"
                          + db_->escapeString(deviceId) + "' LIMIT 1";

        auto resultOpt = db_->makeTransaction()->executeQuery(query);

        if (resultOpt.has_value() && !resultOpt.value().empty()) {

            token = getVariantValue< std::string >(resultOpt.value()[0][0]);
        } else {

            token = "generated_token_" + deviceId;

            std::string createUserQuery = "SELECT add_user('" + db_->escapeString(token) + "')";
            auto userResultOpt = db_->makeTransaction()->executeQuery(createUserQuery);

            if (!userResultOpt.has_value() || userResultOpt.value().empty()) {
                setError(response, ErrorInfo::SERVER_ERROR, "Failed to create user");
                return grpc::Status::OK;
            }

            auto userId = getVariantValue< int32_t >(userResultOpt.value()[0][0]);

            std::string insertDataQuery = "INSERT INTO users_data (user_id, device_id, device_name) VALUES (" + std::to_string(userId) + ", '" + db_->escapeString(deviceId) + "', '" + db_->escapeString(deviceName) + "')";

            db_->makeTransaction()->executeQuery(insertDataQuery);
        }

        response->set_token(token);
        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Authentication error", e.what());
        return grpc::Status::OK;
    }
}

grpc::Status FinanceServiceImpl::ProcessQRCode(grpc::ServerContext * /*context*/, const QRCodeRequest * request, ReceiptDetailsResponse * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        std::string date;
        int32_t sum;
        int64_t fn, fd, fp;
        int32_t type;

        Receipt receipt;
        bool parsed = false;
        if (request->has_qr_code_content()) {
            try {
                receipt = wallet::parseQRDataFromString(request->qr_code_content());
                parsed = true;
            } catch (...) {
            }
        } else if (request->has_receipt()) {
            receipt = request->receipt();
            parsed = true;
        }
        date = receipt.t();

        sum = static_cast< int32_t >(receipt.s() * 100);
        fn = receipt.fn();
        fd = receipt.i();
        fp = receipt.fp();
        type = receipt.n();

        if (!parsed) {
            setError(response, ErrorInfo::PARSING_ERROR, "Failed to parse receipt data");
            return grpc::Status::OK;
        }

        std::string addReceiptQuery = "SELECT add_receipt('" + db_->escapeString(date) + "', " + std::to_string(sum) + ", " + std::to_string(fn) + ", " + std::to_string(fd) + ", " + std::to_string(fp) + ", " + std::to_string(type) + ")";

        auto transaction = db_->makeTransaction();
        auto receiptResultOpt = transaction->executeQuery(addReceiptQuery);

        if (!receiptResultOpt.has_value() || receiptResultOpt.value().empty()) {
            setError(response, ErrorInfo::SERVER_ERROR, "Failed to add receipt");
            return grpc::Status::OK;
        }

        auto receiptId = getVariantValue< int32_t >(receiptResultOpt.value()[0][0]);

        std::string linkQuery = "SELECT link_receipt_to_user(" + std::to_string(userId) + ", " + std::to_string(receiptId) + ")";

        transaction->executeQuery(linkQuery);

        std::string requestQuery = "SELECT create_receipt_request(" + std::to_string(receiptId) + ")";

        auto requestResultOpt = transaction->executeQuery(requestQuery);

        if (!requestResultOpt.has_value() || requestResultOpt.value().empty()) {
            setError(response, ErrorInfo::SERVER_ERROR, "Failed to create receipt request");
            return grpc::Status::OK;
        }

        if (request->create_transaction()) {
            std::string comment = request->has_comment() ? request->comment() : "";
            int32_t categoryId = request->has_category_id() ? request->category_id() : 0;

            std::string createTransactionQuery = "SELECT create_transaction_from_receipt(" + std::to_string(userId) + ", " + std::to_string(receiptId) + ", " + (categoryId > 0 ? std::to_string(categoryId) : "NULL") + ", '" + db_->escapeString(comment) + "')";

            transaction->executeQuery(createTransactionQuery);
        }

        auto * receiptData = response->mutable_receipt_data();

        auto * receiptProto = receiptData->mutable_receipt();
        receiptProto->set_id(receiptId);
        receiptProto->set_t(date);
        receiptProto->set_s(sum / 100.0);
        receiptProto->set_fn(fn);
        receiptProto->set_i(fd);
        receiptProto->set_fp(fp);
        receiptProto->set_n(type);

        std::string query = "SELECT rd.retailer_name, rd.retailer_place, rd.retailer_inn, rd.retailer_address, "
                            "rd.id as receipt_data_id "
                            "FROM receipt_data rd "
                            "WHERE rd.receipt_id = "
                          + std::to_string(receiptId);

        auto resultOpt = transaction->executeQuery(query);

        if (resultOpt.has_value() && !resultOpt.value().empty()) {
            const auto & row = resultOpt.value()[0];

            auto * retailer = receiptData->mutable_retailer();

            if (row[0].index() != std::variant_npos) {
                retailer->set_name(getVariantValue< std::string >(row[0]));
            }
            if (row[1].index() != std::variant_npos) {
                retailer->set_place(getVariantValue< std::string >(row[1]));
            }
            if (row[2].index() != std::variant_npos) {
                retailer->set_inn(getVariantValue< std::string >(row[2]));
            }
            if (row[3].index() != std::variant_npos) {
                retailer->set_address(getVariantValue< std::string >(row[3]));
            }

            int32_t receiptDataId = 0;
            if (row[4].index() != std::variant_npos) {
                receiptDataId = getVariantValue< int32_t >(row[4]);
            }

            if (receiptDataId > 0) {
                std::string itemsQuery = "SELECT id, name, price, quantity, sum, nds_type, payment_type, product_type, measurement_unit "
                                         "FROM receipt_items "
                                         "WHERE receipt_data_id = "
                                       + std::to_string(receiptDataId);

                auto itemsResultOpt = transaction->executeQuery(itemsQuery);

                if (itemsResultOpt.has_value()) {
                    for (const auto & itemRow: itemsResultOpt.value()) {
                        auto * item = receiptData->add_items();

                        if (itemRow[0].index() != std::variant_npos) {
                            item->set_item_id(getVariantValue< uint64_t >(itemRow[0]));
                        }

                        item->set_name(getVariantValue< std::string >(itemRow[1]));
                        item->set_price(getVariantValue< double >(itemRow[2]) / 100.0);
                        item->set_quantity(getVariantValue< double >(itemRow[3]));
                        item->set_sum(getVariantValue< double >(itemRow[4]) / 100.0);

                        item->set_nds_type(static_cast< wallet::ReceiptItem::ENDSType >(getVariantValue< int32_t >(itemRow[5])));
                        item->set_payment_type(static_cast< wallet::ReceiptItem::EPaymentType >(getVariantValue< int32_t >(itemRow[6])));
                        item->set_product_type(static_cast< wallet::ReceiptItem::EProductType >(getVariantValue< int32_t >(itemRow[7])));
                        item->set_measurement_unit(static_cast< wallet::ReceiptItem::EMeasurementUnit >(getVariantValue< int32_t >(itemRow[8])));
                    }
                }
            }
        }

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

grpc::Status FinanceServiceImpl::GetTransactions(grpc::ServerContext * /*context*/, const GetTransactionsRequest * request, TransactionsResponse * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        std::string fromDate, toDate;
        if (request->has_from_date()) {
            fromDate = TimeUtil::ToString(request->from_date());
        }
        if (request->has_to_date()) {
            toDate = TimeUtil::ToString(request->to_date());
        }

        int32_t type = request->has_type() ? request->type() : -1;
        int32_t categoryId = request->has_category_id() ? request->category_id() : 0;
        int32_t limit = request->has_limit() ? request->limit() : 50;
        int32_t offset = request->has_offset() ? request->offset() : 0;

        std::stringstream sql;
        sql << "SELECT t.id, t.timestamp, t.type, t.amount, t.category_id, c.name, "
               "t.receipt_id, t.comment, "
               "EXISTS(SELECT 1 FROM transaction_splits ts WHERE ts.transaction_id = t.id) "
               "FROM transactions t "
               "LEFT JOIN categories c ON c.id = t.category_id "
               "WHERE t.user_id = "
            << userId << " ";

        if (!fromDate.empty()) {
            sql << "AND t.timestamp >= '" << db_->escapeString(fromDate) << "' ";
        }

        if (!toDate.empty()) {
            sql << "AND t.timestamp <= '" << db_->escapeString(toDate) << "' ";
        }

        if (type >= 0) {
            sql << "AND t.type = " << type << " ";
        }

        if (categoryId > 0) {
            sql << "AND t.category_id = " << categoryId << " ";
        }

        sql << "ORDER BY t.timestamp DESC ";
        sql << "LIMIT " << limit << " OFFSET " << offset;

        auto transaction = db_->makeTransaction();
        auto resultOpt = transaction->executeQuery(sql.str());

        auto * transactionsList = response->mutable_transactions();

        if (resultOpt.has_value()) {
            for (const auto & row: resultOpt.value()) {
                auto * transaction = transactionsList->add_transactions();
                transaction->set_id(getVariantValue< int32_t >(row[0]));

                auto * timestamp = transaction->mutable_timestamp();
                *timestamp = stringToProtoTimestamp(getVariantValue< std::string >(row[1]));

                transaction->set_type(getVariantValue< int32_t >(row[2]));
                transaction->set_amount(getVariantValue< int32_t >(row[3]));

                if (row[4].index() != std::variant_npos) {
                    transaction->set_category_id(getVariantValue< int32_t >(row[4]));
                }

                if (row[5].index() != std::variant_npos) {
                    transaction->set_category_name(getVariantValue< std::string >(row[5]));
                }

                if (row[6].index() != std::variant_npos) {
                    transaction->set_receipt_id(getVariantValue< int32_t >(row[6]));
                }

                if (row[7].index() != std::variant_npos) {
                    transaction->set_comment(getVariantValue< std::string >(row[7]));
                }

                transaction->set_has_splits(getVariantValue< bool >(row[8]));
            }
        }

        std::stringstream countSql;
        countSql << "SELECT COUNT(*) FROM transactions t WHERE t.user_id = " << userId << " ";

        if (!fromDate.empty()) {
            countSql << "AND t.timestamp >= '" << db_->escapeString(fromDate) << "' ";
        }

        if (!toDate.empty()) {
            countSql << "AND t.timestamp <= '" << db_->escapeString(toDate) << "' ";
        }

        if (type >= 0) {
            countSql << "AND t.type = " << type << " ";
        }

        if (categoryId > 0) {
            countSql << "AND t.category_id = " << categoryId << " ";
        }

        auto countResultOpt = transaction->executeQuery(countSql.str());
        if (countResultOpt.has_value() && !countResultOpt.value().empty()) {
            transactionsList->set_total_count(getVariantValue< int32_t >(countResultOpt.value()[0][0]));
        }

        std::stringstream statsSql;
        statsSql << "SELECT "
                    "COALESCE(SUM(CASE WHEN type = 0 THEN amount ELSE 0 END), 0) as total_income, "
                    "COALESCE(SUM(CASE WHEN type = 1 THEN amount ELSE 0 END), 0) as total_expense "
                    "FROM transactions t "
                    "WHERE t.user_id = "
                 << userId << " ";

        if (!fromDate.empty()) {
            statsSql << "AND t.timestamp >= '" << db_->escapeString(fromDate) << "' ";
        }

        if (!toDate.empty()) {
            statsSql << "AND t.timestamp <= '" << db_->escapeString(toDate) << "' ";
        }

        if (type >= 0) {
            statsSql << "AND t.type = " << type << " ";
        }

        if (categoryId > 0) {
            statsSql << "AND t.category_id = " << categoryId << " ";
        }

        auto statsResultOpt = transaction->executeQuery(statsSql.str());
        if (statsResultOpt.has_value() && !statsResultOpt.value().empty()) {
            auto totalIncome = getVariantValue< int32_t >(statsResultOpt.value()[0][0]);
            auto totalExpense = getVariantValue< int32_t >(statsResultOpt.value()[0][1]);
            int32_t balance = totalIncome - totalExpense;

            transactionsList->set_total_income(totalIncome);
            transactionsList->set_total_expense(totalExpense);
            transactionsList->set_balance(balance);
        }

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

grpc::Status FinanceServiceImpl::GetReceipts(grpc::ServerContext * /*context*/, const GetReceiptsRequest * request, ReceiptsResponse * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        std::string fromDate, toDate;
        if (request->has_from_date()) {
            fromDate = TimeUtil::ToString(request->from_date());
        }
        if (request->has_to_date()) {
            toDate = TimeUtil::ToString(request->to_date());
        }

        int32_t limit = request->has_limit() ? request->limit() : 50;
        int32_t offset = request->has_offset() ? request->offset() : 0;

        std::stringstream sql;
        sql << "SELECT r.id, r.t, r.s, r.n, rd.retailer_name, "
               "(SELECT COUNT(*) FROM receipt_items ri WHERE ri.receipt_data_id = rd.id), "
               "EXISTS(SELECT 1 FROM transactions t WHERE t.receipt_id = r.id) "
               "FROM receipts r "
               "JOIN user_receipts ur ON ur.receipt_id = r.id "
               "LEFT JOIN receipt_data rd ON rd.receipt_id = r.id "
               "WHERE ur.user_id = "
            << userId << " ";

        if (!fromDate.empty()) {
            sql << "AND r.created_at >= '" << db_->escapeString(fromDate) << "' ";
        }

        if (!toDate.empty()) {
            sql << "AND r.created_at <= '" << db_->escapeString(toDate) << "' ";
        }

        sql << "ORDER BY r.t DESC ";
        sql << "LIMIT " << limit << " OFFSET " << offset;

        auto transaction = db_->makeTransaction();
        auto resultOpt = transaction->executeQuery(sql.str());

        auto * receiptsList = response->mutable_receipts();

        if (resultOpt.has_value()) {
            for (const auto & row: resultOpt.value()) {
                auto * receipt = receiptsList->add_receipts();
                receipt->set_id(getVariantValue< int32_t >(row[0]));
                receipt->set_date(getVariantValue< std::string >(row[1]));
                receipt->set_sum(getVariantValue< int32_t >(row[2]));
                receipt->set_receipt_type(getVariantValue< int32_t >(row[3]));

                if (row[4].index() != std::variant_npos) {
                    receipt->set_retailer_name(getVariantValue< std::string >(row[4]));
                }

                receipt->set_items_count(getVariantValue< int32_t >(row[5]));
                receipt->set_has_transaction(getVariantValue< bool >(row[6]));
            }
        }

        std::string countQuery = "SELECT COUNT(*) FROM user_receipts WHERE user_id = " + std::to_string(userId);
        auto countResultOpt = transaction->executeQuery(countQuery);

        if (countResultOpt.has_value() && !countResultOpt.value().empty()) {
            receiptsList->set_total_count(getVariantValue< int32_t >(countResultOpt.value()[0][0]));
        }

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

grpc::Status FinanceServiceImpl::GetReceiptDetails(grpc::ServerContext * /*context*/, const GetReceiptDetailsRequest * request, ReceiptDetailsResponse * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        int32_t receiptId = request->receipt_id();

        std::string accessQuery = "SELECT 1 FROM user_receipts WHERE user_id = " + std::to_string(userId) + " AND receipt_id = " + std::to_string(receiptId);

        auto accessResultOpt = db_->makeTransaction()->executeQuery(accessQuery);

        if (!accessResultOpt.has_value() || accessResultOpt.value().empty()) {
            setError(response, ErrorInfo::NOT_FOUND, "Receipt not found or access denied");
            return grpc::Status::OK;
        }

        std::string query = "SELECT r.id, r.t, r.s, r.fn, r.i, r.fp, r.n, "
                            "rd.retailer_name, rd.retailer_place, rd.retailer_inn, rd.retailer_address, "
                            "rd.id as receipt_data_id "
                            "FROM receipts r "
                            "LEFT JOIN receipt_data rd ON rd.receipt_id = r.id "
                            "WHERE r.id = "
                          + std::to_string(receiptId);

        auto resultOpt = db_->makeTransaction()->executeQuery(query);

        if (!resultOpt.has_value() || resultOpt.value().empty()) {
            setError(response, ErrorInfo::NOT_FOUND, "Receipt details not found");
            return grpc::Status::OK;
        }

        const auto & row = resultOpt.value()[0];

        auto * receiptData = response->mutable_receipt_data();

        auto * receiptProto = receiptData->mutable_receipt();
        receiptProto->set_id(receiptId);
        receiptProto->set_t(getVariantValue< std::string >(row[1]));
        receiptProto->set_s(getVariantValue< double >(row[2]) / 100.0);
        receiptProto->set_fn(getVariantValue< uint64_t >(row[3]));
        receiptProto->set_i(getVariantValue< uint64_t >(row[4]));
        receiptProto->set_fp(getVariantValue< uint64_t >(row[5]));
        receiptProto->set_n(getVariantValue< int32_t >(row[6]));

        auto * retailer = receiptData->mutable_retailer();

        if (row[7].index() != std::variant_npos) {
            retailer->set_name(getVariantValue< std::string >(row[7]));
        }
        if (row[8].index() != std::variant_npos) {
            retailer->set_place(getVariantValue< std::string >(row[8]));
        }
        if (row[9].index() != std::variant_npos) {
            retailer->set_inn(getVariantValue< std::string >(row[9]));
        }
        if (row[10].index() != std::variant_npos) {
            retailer->set_address(getVariantValue< std::string >(row[10]));
        }

        int32_t receiptDataId = 0;
        if (row[11].index() != std::variant_npos) {
            receiptDataId = getVariantValue< int32_t >(row[11]);
        }

        if (receiptDataId > 0) {
            std::string itemsQuery = "SELECT id, name, price, quantity, sum, nds_type, payment_type, product_type, measurement_unit "
                                     "FROM receipt_items "
                                     "WHERE receipt_data_id = "
                                   + std::to_string(receiptDataId);

            auto itemsResultOpt = db_->makeTransaction()->executeQuery(itemsQuery);

            if (itemsResultOpt.has_value()) {
                for (const auto & itemRow: itemsResultOpt.value()) {
                    auto * item = receiptData->add_items();

                    if (itemRow[0].index() != std::variant_npos) {
                        item->set_item_id(getVariantValue< uint64_t >(itemRow[0]));
                    }

                    item->set_name(getVariantValue< std::string >(itemRow[1]));
                    item->set_price(getVariantValue< double >(itemRow[2]) / 100.0);
                    item->set_quantity(getVariantValue< double >(itemRow[3]));
                    item->set_sum(getVariantValue< double >(itemRow[4]) / 100.0);

                    item->set_nds_type(static_cast< wallet::ReceiptItem::ENDSType >(getVariantValue< int32_t >(itemRow[5])));
                    item->set_payment_type(static_cast< wallet::ReceiptItem::EPaymentType >(getVariantValue< int32_t >(itemRow[6])));
                    item->set_product_type(static_cast< wallet::ReceiptItem::EProductType >(getVariantValue< int32_t >(itemRow[7])));
                    item->set_measurement_unit(static_cast< wallet::ReceiptItem::EMeasurementUnit >(getVariantValue< int32_t >(itemRow[8])));
                }
            }
        }

        std::string transactionQuery = "SELECT id FROM transactions WHERE receipt_id = " + std::to_string(receiptId) + " LIMIT 1";

        auto transactionResultOpt = db_->makeTransaction()->executeQuery(transactionQuery);

        if (transactionResultOpt.has_value() && !transactionResultOpt.value().empty()) {
            auto transactionId = getVariantValue< int32_t >(transactionResultOpt.value()[0][0]);
            auto * additionalInfo = receiptData->mutable_additional_info();
            (*additionalInfo)["transaction_id"] = std::to_string(transactionId);
        }

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

grpc::Status FinanceServiceImpl::CreateTransaction(grpc::ServerContext * /*context*/, const CreateTransactionRequest * request, CreateTransactionResponse * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        const auto & transaction = request->transaction();

        if (transaction.type() != 0 && transaction.type() != 1) {
            setError(response, ErrorInfo::INVALID_REQUEST, "Invalid transaction type");
            return grpc::Status::OK;
        }

        if (transaction.amount() <= 0) {
            setError(response, ErrorInfo::INVALID_REQUEST, "Amount must be positive");
            return grpc::Status::OK;
        }

        std::stringstream sql;
        sql << "SELECT add_transaction(" << userId << ", "
            << transaction.type() << ", "
            << transaction.amount() << ", '"
            << db_->escapeString(TimeUtil::ToString(transaction.timestamp())) << "'";

        if (transaction.has_category_id()) {
            sql << ", " << transaction.category_id();
        } else {
            sql << ", NULL";
        }

        if (transaction.has_receipt_id()) {
            sql << ", " << transaction.receipt_id();
        } else {
            sql << ", NULL";
        }

        if (transaction.has_comment()) {
            sql << ", '" << db_->escapeString(transaction.comment()) << "'";
        } else {
            sql << ", NULL";
        }

        sql << ")";

        auto resultOpt = db_->makeTransaction()->executeQuery(sql.str());

        if (!resultOpt.has_value() || resultOpt.value().empty()) {
            setError(response, ErrorInfo::SERVER_ERROR, "Failed to create transaction");
            return grpc::Status::OK;
        }

        auto transactionId = getVariantValue< int32_t >(resultOpt.value()[0][0]);
        response->set_transaction_id(transactionId);

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

grpc::Status FinanceServiceImpl::UpdateTransaction(grpc::ServerContext * /*context*/, const UpdateTransactionRequest * request, Response * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        const auto & transaction = request->transaction();

        if (!transaction.has_id()) {
            setError(response, ErrorInfo::INVALID_REQUEST, "Transaction ID is required");
            return grpc::Status::OK;
        }

        if (transaction.type() != 0 && transaction.type() != 1) {
            setError(response, ErrorInfo::INVALID_REQUEST, "Invalid transaction type");
            return grpc::Status::OK;
        }

        if (transaction.amount() <= 0) {
            setError(response, ErrorInfo::INVALID_REQUEST, "Amount must be positive");
            return grpc::Status::OK;
        }

        std::string accessQuery = "SELECT 1 FROM transactions WHERE id = " + std::to_string(transaction.id()) + " AND user_id = " + std::to_string(userId);

        auto accessResultOpt = db_->makeTransaction()->executeQuery(accessQuery);

        if (!accessResultOpt.has_value() || accessResultOpt.value().empty()) {
            setError(response, ErrorInfo::NOT_FOUND, "Transaction not found or access denied");
            return grpc::Status::OK;
        }

        std::stringstream sql;
        sql << "UPDATE transactions SET "
               "type = "
            << transaction.type() << ", "
                                     "amount = "
            << transaction.amount() << ", "
                                       "timestamp = '"
            << db_->escapeString(TimeUtil::ToString(transaction.timestamp())) << "'";

        if (transaction.has_category_id()) {
            sql << ", category_id = " << transaction.category_id();
        } else {
            sql << ", category_id = NULL";
        }

        if (transaction.has_receipt_id()) {
            sql << ", receipt_id = " << transaction.receipt_id();
        } else {
            sql << ", receipt_id = NULL";
        }

        if (transaction.has_comment()) {
            sql << ", comment = '" << db_->escapeString(transaction.comment()) << "'";
        } else {
            sql << ", comment = NULL";
        }

        sql << " WHERE id = " << transaction.id();

        db_->makeTransaction()->executeQuery(sql.str());

        response->mutable_success();

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

grpc::Status FinanceServiceImpl::DeleteTransaction(grpc::ServerContext * /*context*/, const DeleteTransactionRequest * request, Response * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        int32_t transactionId = request->transaction_id();

        std::string accessQuery = "SELECT 1 FROM transactions WHERE id = " + std::to_string(transactionId) + " AND user_id = " + std::to_string(userId);

        auto accessResultOpt = db_->makeTransaction()->executeQuery(accessQuery);

        if (!accessResultOpt.has_value() || accessResultOpt.value().empty()) {
            setError(response, ErrorInfo::NOT_FOUND, "Transaction not found or access denied");
            return grpc::Status::OK;
        }

        auto transaction = db_->makeTransaction();

        std::string deleteSplitsQuery = "DELETE FROM transaction_splits WHERE transaction_id = " + std::to_string(transactionId);

        transaction->executeQuery(deleteSplitsQuery);

        std::string deleteTransactionQuery = "DELETE FROM transactions WHERE id = " + std::to_string(transactionId);

        transaction->executeQuery(deleteTransactionQuery);

        response->mutable_success();

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

bool FinanceServiceImpl::getTransactionData(int32_t transactionId, TransactionDetails * details) {
    try {

        std::string query = "SELECT t.id, t.timestamp, t.type, t.amount, t.category_id, c.name, "
                            "t.receipt_id, t.comment "
                            "FROM transactions t "
                            "LEFT JOIN categories c ON c.id = t.category_id "
                            "WHERE t.id = "
                          + std::to_string(transactionId);

        auto resultOpt = db_->makeTransaction()->executeQuery(query);

        if (!resultOpt.has_value() || resultOpt.value().empty()) {
            return false;
        }

        const auto & row = resultOpt.value()[0];

        details->set_id(transactionId);

        auto * timestamp = details->mutable_timestamp();
        *timestamp = stringToProtoTimestamp(getVariantValue< std::string >(row[1]));

        details->set_type(getVariantValue< int32_t >(row[2]));
        details->set_amount(getVariantValue< int32_t >(row[3]));

        if (row[4].index() != std::variant_npos) {
            details->set_category_id(getVariantValue< int32_t >(row[4]));
        }

        if (row[5].index() != std::variant_npos) {
            details->set_category_name(getVariantValue< std::string >(row[5]));
        }

        if (row[6].index() != std::variant_npos) {
            auto receiptId = getVariantValue< int32_t >(row[6]);
            details->set_receipt_id(receiptId);

            std::string receiptQuery = "SELECT r.id, r.t, r.s, r.n, rd.retailer_name, "
                                       "(SELECT COUNT(*) FROM receipt_items ri WHERE ri.receipt_data_id = rd.id) "
                                       "FROM receipts r "
                                       "LEFT JOIN receipt_data rd ON rd.receipt_id = r.id "
                                       "WHERE r.id = "
                                     + std::to_string(receiptId);

            auto receiptResultOpt = db_->makeTransaction()->executeQuery(receiptQuery);

            if (receiptResultOpt.has_value() && !receiptResultOpt.value().empty()) {
                const auto & receiptRow = receiptResultOpt.value()[0];
                auto * receipt = details->mutable_receipt();
                receipt->set_id(getVariantValue< int32_t >(receiptRow[0]));
                receipt->set_date(getVariantValue< std::string >(receiptRow[1]));
                receipt->set_sum(getVariantValue< int32_t >(receiptRow[2]));
                receipt->set_receipt_type(getVariantValue< int32_t >(receiptRow[3]));

                if (receiptRow[4].index() != std::variant_npos) {
                    receipt->set_retailer_name(getVariantValue< std::string >(receiptRow[4]));
                }

                receipt->set_items_count(getVariantValue< int32_t >(receiptRow[5]));
                receipt->set_has_transaction(true);
            }
        }

        if (row[7].index() != std::variant_npos) {
            details->set_comment(getVariantValue< std::string >(row[7]));
        }

        std::string splitsQuery = "SELECT ts.id, ts.character_id, uc.name, ts.amount, ts.comment "
                                  "FROM transaction_splits ts "
                                  "JOIN user_characters uc ON uc.id = ts.character_id "
                                  "WHERE ts.transaction_id = "
                                + std::to_string(transactionId) + " "
                                                                  "ORDER BY ts.id";

        auto splitsResultOpt = db_->makeTransaction()->executeQuery(splitsQuery);

        if (splitsResultOpt.has_value()) {
            for (const auto & splitRow: splitsResultOpt.value()) {
                auto * split = details->add_splits();
                split->set_id(getVariantValue< int32_t >(splitRow[0]));
                split->set_character_id(getVariantValue< int32_t >(splitRow[1]));
                split->set_character_name(getVariantValue< std::string >(splitRow[2]));
                split->set_amount(getVariantValue< int32_t >(splitRow[3]));

                if (splitRow[4].index() != std::variant_npos) {
                    split->set_comment(getVariantValue< std::string >(splitRow[4]));
                }
            }
        }

        return true;
    } catch (const std::exception & e) {

        return false;
    }
}

grpc::Status FinanceServiceImpl::GetTransactionDetails(grpc::ServerContext * /*context*/, const GetTransactionDetailsRequest * request, TransactionDetailsResponse * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        int32_t transactionId = request->transaction_id();

        std::string accessQuery = "SELECT 1 FROM transactions WHERE id = " + std::to_string(transactionId) + " AND user_id = " + std::to_string(userId);

        auto accessResultOpt = db_->makeTransaction()->executeQuery(accessQuery);

        if (!accessResultOpt.has_value() || accessResultOpt.value().empty()) {
            setError(response, ErrorInfo::NOT_FOUND, "Transaction not found or access denied");
            return grpc::Status::OK;
        }

        if (!getTransactionData(transactionId, response->mutable_transaction())) {
            setError(response, ErrorInfo::NOT_FOUND, "Transaction details not found");
            return grpc::Status::OK;
        }

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

grpc::Status FinanceServiceImpl::CreateSplit(grpc::ServerContext * /*context*/, const CreateSplitRequest * request, CreateSplitResponse * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        const auto & split = request->split();

        if (split.transaction_id() <= 0) {
            setError(response, ErrorInfo::INVALID_REQUEST, "Transaction ID is required");
            return grpc::Status::OK;
        }

        if (split.character_id() <= 0) {
            setError(response, ErrorInfo::INVALID_REQUEST, "Character ID is required");
            return grpc::Status::OK;
        }

        if (split.amount() <= 0) {
            setError(response, ErrorInfo::INVALID_REQUEST, "Amount must be positive");
            return grpc::Status::OK;
        }

        std::string transactionQuery = "SELECT 1 FROM transactions WHERE id = " + std::to_string(split.transaction_id()) + " AND user_id = " + std::to_string(userId);

        auto transactionResultOpt = db_->makeTransaction()->executeQuery(transactionQuery);

        if (!transactionResultOpt.has_value() || transactionResultOpt.value().empty()) {
            setError(response, ErrorInfo::NOT_FOUND, "Transaction not found or access denied");
            return grpc::Status::OK;
        }

        std::string characterQuery = "SELECT 1 FROM user_characters WHERE id = " + std::to_string(split.character_id()) + " AND user_id = " + std::to_string(userId);

        auto characterResultOpt = db_->makeTransaction()->executeQuery(characterQuery);

        if (!characterResultOpt.has_value() || characterResultOpt.value().empty()) {
            setError(response, ErrorInfo::NOT_FOUND, "Character not found or access denied");
            return grpc::Status::OK;
        }

        std::stringstream sql;
        sql << "SELECT add_transaction_split("
            << split.transaction_id() << ", "
            << split.character_id() << ", "
            << split.amount();

        if (split.has_comment()) {
            sql << ", '" << db_->escapeString(split.comment()) << "'";
        } else {
            sql << ", NULL";
        }

        sql << ")";

        auto resultOpt = db_->makeTransaction()->executeQuery(sql.str());

        if (!resultOpt.has_value() || resultOpt.value().empty()) {
            setError(response, ErrorInfo::SERVER_ERROR, "Failed to create split");
            return grpc::Status::OK;
        }

        auto splitId = getVariantValue< int32_t >(resultOpt.value()[0][0]);
        response->set_split_id(splitId);

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

grpc::Status FinanceServiceImpl::UpdateSplit(grpc::ServerContext * /*context*/, const UpdateSplitRequest * request, Response * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        const auto & split = request->split();

        if (!split.has_id()) {
            setError(response, ErrorInfo::INVALID_REQUEST, "Split ID is required");
            return grpc::Status::OK;
        }

        if (split.amount() <= 0) {
            setError(response, ErrorInfo::INVALID_REQUEST, "Amount must be positive");
            return grpc::Status::OK;
        }

        std::string accessQuery = "SELECT 1 FROM transaction_splits ts "
                                  "JOIN transactions t ON t.id = ts.transaction_id "
                                  "WHERE ts.id = "
                                + std::to_string(split.id()) + " AND t.user_id = " + std::to_string(userId);

        auto accessResultOpt = db_->makeTransaction()->executeQuery(accessQuery);

        if (!accessResultOpt.has_value() || accessResultOpt.value().empty()) {
            setError(response, ErrorInfo::NOT_FOUND, "Split not found or access denied");
            return grpc::Status::OK;
        }

        std::stringstream sql;
        sql << "UPDATE transaction_splits SET amount = " << split.amount();

        if (split.has_comment()) {
            sql << ", comment = '" << db_->escapeString(split.comment()) << "'";
        }

        sql << " WHERE id = " << split.id();

        db_->makeTransaction()->executeQuery(sql.str());

        response->mutable_success();

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

grpc::Status FinanceServiceImpl::DeleteSplit(grpc::ServerContext * /*context*/, const DeleteSplitRequest * request, Response * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        int32_t splitId = request->split_id();

        std::string accessQuery = "SELECT 1 FROM transaction_splits ts "
                                  "JOIN transactions t ON t.id = ts.transaction_id "
                                  "WHERE ts.id = "
                                + std::to_string(splitId) + " AND t.user_id = " + std::to_string(userId);

        auto accessResultOpt = db_->makeTransaction()->executeQuery(accessQuery);

        if (!accessResultOpt.has_value() || accessResultOpt.value().empty()) {
            setError(response, ErrorInfo::NOT_FOUND, "Split not found or access denied");
            return grpc::Status::OK;
        }

        std::string deleteQuery = "DELETE FROM transaction_splits WHERE id = " + std::to_string(splitId);
        db_->makeTransaction()->executeQuery(deleteQuery);

        response->mutable_success();

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

grpc::Status FinanceServiceImpl::GetCharacters(grpc::ServerContext * /*context*/, const GetCharactersRequest * request, CharactersResponse * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        std::string query = "SELECT id, name FROM user_characters WHERE user_id = " + std::to_string(userId) + " ORDER BY id";

        auto resultOpt = db_->makeTransaction()->executeQuery(query);

        auto * charactersList = response->mutable_characters_list();
        if (resultOpt.has_value()) {
            for (const auto & row: resultOpt.value()) {
                auto * character = charactersList->add_characters();
                character->set_id(getVariantValue< int32_t >(row[0]));
                character->set_name(getVariantValue< std::string >(row[1]));
            }
        }

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

grpc::Status FinanceServiceImpl::ManageCharacter(grpc::ServerContext * /*context*/, const ManageCharacterRequest * request, ManageCharacterResponse * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        if (request->name().empty()) {
            setError(response, ErrorInfo::INVALID_REQUEST, "Character name is required");
            return grpc::Status::OK;
        }

        if (request->has_id()) {

            int32_t characterId = request->id();

            std::string accessQuery = "SELECT 1 FROM user_characters WHERE id = " + std::to_string(characterId) + " AND user_id = " + std::to_string(userId);

            auto accessResultOpt = db_->makeTransaction()->executeQuery(accessQuery);

            if (!accessResultOpt.has_value() || accessResultOpt.value().empty()) {
                setError(response, ErrorInfo::NOT_FOUND, "Character not found or access denied");
                return grpc::Status::OK;
            }

            std::string updateQuery = "UPDATE user_characters SET name = '" + db_->escapeString(request->name()) + "' WHERE id = " + std::to_string(characterId) + " AND user_id = " + std::to_string(userId);

            db_->makeTransaction()->executeQuery(updateQuery);

            response->set_character_id(characterId);
        } else {

            std::string createQuery = "SELECT add_user_character(" + std::to_string(userId) + ", '" + db_->escapeString(request->name()) + "')";

            auto resultOpt = db_->makeTransaction()->executeQuery(createQuery);

            if (!resultOpt.has_value() || resultOpt.value().empty()) {
                setError(response, ErrorInfo::SERVER_ERROR, "Failed to create character");
                return grpc::Status::OK;
            }

            auto characterId = getVariantValue< int32_t >(resultOpt.value()[0][0]);
            response->set_character_id(characterId);
        }

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

grpc::Status FinanceServiceImpl::DeleteCharacter(grpc::ServerContext * /*context*/, const ManageCharacterRequest * request, Response * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        if (!request->has_id()) {
            setError(response, ErrorInfo::INVALID_REQUEST, "Character ID is required");
            return grpc::Status::OK;
        }

        int32_t characterId = request->id();

        std::string accessQuery = "SELECT name FROM user_characters WHERE id = " + std::to_string(characterId) + " AND user_id = " + std::to_string(userId);

        auto accessResultOpt = db_->makeTransaction()->executeQuery(accessQuery);

        if (!accessResultOpt.has_value() || accessResultOpt.value().empty()) {
            setError(response, ErrorInfo::NOT_FOUND, "Character not found or access denied");
            return grpc::Status::OK;
        }

        auto characterName = getVariantValue< std::string >(accessResultOpt.value()[0][0]);
        if (characterName == "Основной") {
            setError(response, ErrorInfo::INVALID_REQUEST, "Cannot delete the main character");
            return grpc::Status::OK;
        }

        auto transaction = db_->makeTransaction();

        std::string deleteSplitsQuery = "DELETE FROM transaction_splits WHERE character_id = " + std::to_string(characterId);

        transaction->executeQuery(deleteSplitsQuery);

        std::string deleteCharacterQuery = "DELETE FROM user_characters WHERE id = " + std::to_string(characterId) + " AND user_id = " + std::to_string(userId);

        transaction->executeQuery(deleteCharacterQuery);

        response->mutable_success();

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

grpc::Status FinanceServiceImpl::GetCategories(grpc::ServerContext * /*context*/, const GetCategoriesRequest * request, CategoriesResponse * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        std::string query = "SELECT id, name FROM categories ORDER BY name";

        auto resultOpt = db_->makeTransaction()->executeQuery(query);

        auto * categoriesList = response->mutable_categories_list();
        if (resultOpt.has_value()) {
            for (const auto & row: resultOpt.value()) {
                auto * category = categoriesList->add_categories();
                category->set_id(getVariantValue< int32_t >(row[0]));
                category->set_name(getVariantValue< std::string >(row[1]));
            }
        }

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

grpc::Status FinanceServiceImpl::ManageCategory(grpc::ServerContext * /*context*/, const ManageCategoryRequest * request, ManageCategoryResponse * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        if (request->name().empty()) {
            setError(response, ErrorInfo::INVALID_REQUEST, "Category name is required");
            return grpc::Status::OK;
        }

        if (request->has_id()) {

            int32_t categoryId = request->id();

            std::string categoryQuery = "SELECT 1 FROM categories WHERE id = " + std::to_string(categoryId);

            auto categoryResultOpt = db_->makeTransaction()->executeQuery(categoryQuery);

            if (!categoryResultOpt.has_value() || categoryResultOpt.value().empty()) {
                setError(response, ErrorInfo::NOT_FOUND, "Category not found");
                return grpc::Status::OK;
            }

            std::string updateQuery = "UPDATE categories SET name = '" + db_->escapeString(request->name()) + "' WHERE id = " + std::to_string(categoryId);

            db_->makeTransaction()->executeQuery(updateQuery);

            response->set_category_id(categoryId);
        } else {

            std::string createQuery = "SELECT add_category('" + db_->escapeString(request->name()) + "')";

            auto resultOpt = db_->makeTransaction()->executeQuery(createQuery);

            if (!resultOpt.has_value() || resultOpt.value().empty()) {
                setError(response, ErrorInfo::SERVER_ERROR, "Failed to create category");
                return grpc::Status::OK;
            }

            auto categoryId = getVariantValue< int32_t >(resultOpt.value()[0][0]);
            response->set_category_id(categoryId);
        }

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

grpc::Status FinanceServiceImpl::DeleteCategory(grpc::ServerContext * /*context*/, const ManageCategoryRequest * request, Response * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        if (!request->has_id()) {
            setError(response, ErrorInfo::INVALID_REQUEST, "Category ID is required");
            return grpc::Status::OK;
        }

        int32_t categoryId = request->id();

        std::string categoryQuery = "SELECT 1 FROM categories WHERE id = " + std::to_string(categoryId);

        auto categoryResultOpt = db_->makeTransaction()->executeQuery(categoryQuery);

        if (!categoryResultOpt.has_value() || categoryResultOpt.value().empty()) {
            setError(response, ErrorInfo::NOT_FOUND, "Category not found");
            return grpc::Status::OK;
        }

        auto transaction = db_->makeTransaction();

        std::string updateTransactionsQuery = "UPDATE transactions SET category_id = NULL WHERE category_id = " + std::to_string(categoryId);

        transaction->executeQuery(updateTransactionsQuery);

        std::string deleteCategoryQuery = "DELETE FROM categories WHERE id = " + std::to_string(categoryId);

        transaction->executeQuery(deleteCategoryQuery);

        response->mutable_success();

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

grpc::Status FinanceServiceImpl::GetStatistics(grpc::ServerContext * /*context*/, const GetStatisticsRequest * request, StatisticsResponse * response) {
    try {

        int32_t userId;
        if (!authenticateUser(request->auth().token(), userId)) {
            setError(response, ErrorInfo::UNAUTHORIZED, "Invalid token");
            return grpc::Status::OK;
        }

        std::string fromDate, toDate;
        if (request->has_from_date()) {
            fromDate = TimeUtil::ToString(request->from_date());
        }
        if (request->has_to_date()) {
            toDate = TimeUtil::ToString(request->to_date());
        }

        auto * statistics = response->mutable_statistics();

        std::stringstream sql;
        sql << "SELECT "
               "COALESCE(SUM(CASE WHEN type = 0 THEN amount ELSE 0 END), 0) as total_income, "
               "COALESCE(SUM(CASE WHEN type = 1 THEN amount ELSE 0 END), 0) as total_expense, "
               "COUNT(CASE WHEN type = 0 THEN 1 END) as income_count, "
               "COUNT(CASE WHEN type = 1 THEN 1 END) as expense_count "
               "FROM transactions "
               "WHERE user_id = "
            << userId;

        if (!fromDate.empty()) {
            sql << " AND timestamp >= '" << db_->escapeString(fromDate) << "'";
        }

        if (!toDate.empty()) {
            sql << " AND timestamp <= '" << db_->escapeString(toDate) << "'";
        }

        auto transaction = db_->makeTransaction();
        auto resultOpt = transaction->executeQuery(sql.str());

        if (resultOpt.has_value() && !resultOpt.value().empty()) {
            const auto & row = resultOpt.value()[0];
            auto totalIncome = getVariantValue< int32_t >(row[0]);
            auto totalExpense = getVariantValue< int32_t >(row[1]);
            auto incomeCount = getVariantValue< int32_t >(row[2]);
            auto expenseCount = getVariantValue< int32_t >(row[3]);

            statistics->set_total_income(totalIncome);
            statistics->set_total_expense(totalExpense);
            statistics->set_balance(totalIncome - totalExpense);
            statistics->set_income_transactions_count(incomeCount);
            statistics->set_expense_transactions_count(expenseCount);
        }

        auto * chartData = statistics->mutable_chart_data();

        std::stringstream dailySql;
        dailySql << "SELECT "
                    "TO_CHAR(timestamp, 'YYYY-MM-DD') as date, "
                    "COALESCE(SUM(CASE WHEN type = 0 THEN amount ELSE 0 END), 0) as daily_income, "
                    "COALESCE(SUM(CASE WHEN type = 1 THEN amount ELSE 0 END), 0) as daily_expense "
                    "FROM transactions "
                    "WHERE user_id = "
                 << userId;

        if (!fromDate.empty()) {
            dailySql << " AND timestamp >= '" << db_->escapeString(fromDate) << "'";
        }

        if (!toDate.empty()) {
            dailySql << " AND timestamp <= '" << db_->escapeString(toDate) << "'";
        }

        dailySql << " GROUP BY TO_CHAR(timestamp, 'YYYY-MM-DD') "
                    "ORDER BY TO_CHAR(timestamp, 'YYYY-MM-DD')";

        auto dailyResultOpt = transaction->executeQuery(dailySql.str());

        if (dailyResultOpt.has_value()) {
            for (const auto & row: dailyResultOpt.value()) {
                auto * dailyData = chartData->add_daily();
                dailyData->set_date(getVariantValue< std::string >(row[0]));
                dailyData->set_income(getVariantValue< int32_t >(row[1]));
                dailyData->set_expense(getVariantValue< int32_t >(row[2]));
            }
        }

        std::stringstream expenseCategorySql;
        expenseCategorySql << "SELECT "
                              "t.category_id, "
                              "COALESCE(c.name, 'Без категории') as category_name, "
                              "COUNT(t.id) as transactions_count, "
                              "SUM(t.amount) as total_amount "
                              "FROM transactions t "
                              "LEFT JOIN categories c ON c.id = t.category_id "
                              "WHERE t.user_id = "
                           << userId << " AND t.type = 1";

        if (!fromDate.empty()) {
            expenseCategorySql << " AND t.timestamp >= '" << db_->escapeString(fromDate) << "'";
        }

        if (!toDate.empty()) {
            expenseCategorySql << " AND t.timestamp <= '" << db_->escapeString(toDate) << "'";
        }

        expenseCategorySql << " GROUP BY t.category_id, c.name "
                              "ORDER BY total_amount DESC";

        auto expenseCategoryResultOpt = transaction->executeQuery(expenseCategorySql.str());

        int32_t totalExpenseAmount = 0;
        if (expenseCategoryResultOpt.has_value()) {
            for (const auto & row: expenseCategoryResultOpt.value()) {
                totalExpenseAmount += getVariantValue< int32_t >(row[3]);
            }
        }

        if (expenseCategoryResultOpt.has_value()) {
            for (const auto & row: expenseCategoryResultOpt.value()) {
                auto * categoryStats = chartData->add_expenses_by_category();

                categoryStats->set_category_id(row[0].index() != std::variant_npos ? getVariantValue< int32_t >(row[0]) : 0);

                categoryStats->set_category_name(getVariantValue< std::string >(row[1]));
                categoryStats->set_transactions_count(getVariantValue< int32_t >(row[2]));
                categoryStats->set_total_amount(getVariantValue< int32_t >(row[3]));

                double percentage = 0.0;
                if (totalExpenseAmount > 0) {
                    percentage = (static_cast< double >(getVariantValue< int32_t >(row[3])) / totalExpenseAmount) * 100.0;
                }
                categoryStats->set_percentage(percentage);
            }
        }

        std::stringstream incomeCategorySql;
        incomeCategorySql << "SELECT "
                             "t.category_id, "
                             "COALESCE(c.name, 'Без категории') as category_name, "
                             "COUNT(t.id) as transactions_count, "
                             "SUM(t.amount) as total_amount "
                             "FROM transactions t "
                             "LEFT JOIN categories c ON c.id = t.category_id "
                             "WHERE t.user_id = "
                          << userId << " AND t.type = 0";

        if (!fromDate.empty()) {
            incomeCategorySql << " AND t.timestamp >= '" << db_->escapeString(fromDate) << "'";
        }

        if (!toDate.empty()) {
            incomeCategorySql << " AND t.timestamp <= '" << db_->escapeString(toDate) << "'";
        }

        incomeCategorySql << " GROUP BY t.category_id, c.name "
                             "ORDER BY total_amount DESC";

        auto incomeCategoryResultOpt = transaction->executeQuery(incomeCategorySql.str());

        int32_t totalIncomeAmount = 0;
        if (incomeCategoryResultOpt.has_value()) {
            for (const auto & row: incomeCategoryResultOpt.value()) {
                totalIncomeAmount += getVariantValue< int32_t >(row[3]);
            }
        }

        if (incomeCategoryResultOpt.has_value()) {
            for (const auto & row: incomeCategoryResultOpt.value()) {
                auto * categoryStats = chartData->add_incomes_by_category();

                categoryStats->set_category_id(row[0].index() != std::variant_npos ? getVariantValue< int32_t >(row[0]) : 0);

                categoryStats->set_category_name(getVariantValue< std::string >(row[1]));
                categoryStats->set_transactions_count(getVariantValue< int32_t >(row[2]));
                categoryStats->set_total_amount(getVariantValue< int32_t >(row[3]));

                double percentage = 0.0;
                if (totalIncomeAmount > 0) {
                    percentage = (static_cast< double >(getVariantValue< int32_t >(row[3])) / totalIncomeAmount) * 100.0;
                }
                categoryStats->set_percentage(percentage);
            }
        }

        std::stringstream characterSql;
        characterSql << "SELECT "
                        "uc.id as character_id, "
                        "uc.name as character_name, "
                        "COUNT(ts.id) as splits_count, "
                        "SUM(ts.amount) as total_amount "
                        "FROM user_characters uc "
                        "JOIN transaction_splits ts ON ts.character_id = uc.id "
                        "JOIN transactions t ON t.id = ts.transaction_id "
                        "WHERE uc.user_id = "
                     << userId << " AND t.type = 1";

        if (!fromDate.empty()) {
            characterSql << " AND t.timestamp >= '" << db_->escapeString(fromDate) << "'";
        }

        if (!toDate.empty()) {
            characterSql << " AND t.timestamp <= '" << db_->escapeString(toDate) << "'";
        }

        characterSql << " GROUP BY uc.id, uc.name "
                        "ORDER BY total_amount DESC";

        auto characterResultOpt = transaction->executeQuery(characterSql.str());

        int32_t totalCharacterAmount = 0;
        if (characterResultOpt.has_value()) {
            for (const auto & row: characterResultOpt.value()) {
                totalCharacterAmount += getVariantValue< int32_t >(row[3]);
            }
        }

        if (characterResultOpt.has_value()) {
            for (const auto & row: characterResultOpt.value()) {
                auto * characterStats = chartData->add_expenses_by_character();
                characterStats->set_character_id(getVariantValue< int32_t >(row[0]));
                characterStats->set_character_name(getVariantValue< std::string >(row[1]));
                characterStats->set_splits_count(getVariantValue< int32_t >(row[2]));
                characterStats->set_total_amount(getVariantValue< int32_t >(row[3]));

                double percentage = 0.0;
                if (totalCharacterAmount > 0) {
                    percentage = (static_cast< double >(getVariantValue< int32_t >(row[3])) / totalCharacterAmount) * 100.0;
                }
                characterStats->set_percentage(percentage);
            }
        }

        return grpc::Status::OK;
    } catch (const std::exception & e) {
        setError(response, ErrorInfo::SERVER_ERROR, "Server error", e.what());
        return grpc::Status::OK;
    }
}

template < typename ResponseType >
void FinanceServiceImpl::setError(ResponseType * response, ErrorInfo::ErrorCode code, const std::string & message, const std::string & details) {
    auto * errorInfo = new ErrorInfo();
    errorInfo->set_code(code);
    errorInfo->set_message(message);
    if (!details.empty()) {
        errorInfo->set_details(details);
    }

    if constexpr (std::is_same_v< ResponseType, AuthResponse >) {
        response->set_allocated_error(errorInfo);
    } else if constexpr (std::is_same_v< ResponseType, ReceiptDetailsResponse >) {
        response->set_allocated_error(errorInfo);
    } else if constexpr (std::is_same_v< ResponseType, ReceiptsResponse >) {
        response->set_allocated_error(errorInfo);
    } else if constexpr (std::is_same_v< ResponseType, CreateTransactionResponse >) {
        response->set_allocated_error(errorInfo);
    } else if constexpr (std::is_same_v< ResponseType, TransactionsResponse >) {
        response->set_allocated_error(errorInfo);
    } else if constexpr (std::is_same_v< ResponseType, TransactionDetailsResponse >) {
        response->set_allocated_error(errorInfo);
    } else if constexpr (std::is_same_v< ResponseType, CreateSplitResponse >) {
        response->set_allocated_error(errorInfo);
    } else if constexpr (std::is_same_v< ResponseType, CharactersResponse >) {
        response->set_allocated_error(errorInfo);
    } else if constexpr (std::is_same_v< ResponseType, ManageCharacterResponse >) {
        response->set_allocated_error(errorInfo);
    } else if constexpr (std::is_same_v< ResponseType, CategoriesResponse >) {
        response->set_allocated_error(errorInfo);
    } else if constexpr (std::is_same_v< ResponseType, ManageCategoryResponse >) {
        response->set_allocated_error(errorInfo);
    } else if constexpr (std::is_same_v< ResponseType, StatisticsResponse >) {
        response->set_allocated_error(errorInfo);
    } else if constexpr (std::is_same_v< ResponseType, Response >) {
        response->set_allocated_error(errorInfo);
    } else {
        delete errorInfo;
    }
}
