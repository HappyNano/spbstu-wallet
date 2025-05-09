#include <backend/service/service.h>
#include <utils/database/mock/mock_database.h>
#include <utils/database/mock/mock_transaction.h>

#include <gtest/gtest.h>

#include <memory>
#include <string>

using namespace cxx;
using namespace receipt;
using namespace testing;

namespace {

    class ReceiptScannerServiceTest: public ::Test {
    public:
        void SetUp() override {
            EXPECT_CALL(*mockDb_, makeTransaction)
             .WillRepeatedly([this]() {
                 return mockTransaction_;
             });
            service_ = std::make_unique< receipt::ReceiptScannerServiceImpl >(mockDb_);
        }

    protected:
        const std::shared_ptr< MockDatabase > mockDb_ = std::make_shared< NiceMock< MockDatabase > >();
        const std::shared_ptr< MockTransaction > mockTransaction_ = std::make_shared< NiceMock< MockTransaction > >();

        std::unique_ptr< receipt::ReceiptScannerServiceImpl > service_;
    };

    TEST_F(ReceiptScannerServiceTest, ProcessQRCodeValidInput) {
        // Arrange
        grpc::ServerContext context;
        QRCodeRequest request;
        ReceiptResponse response;

        request.set_qr_code_content("t=20200727T174700&s=432.00&fn=9284000100287274&i=28889&fp=3906849540&n=1");

        // Receipt expectedReceipt;
        // expectedReceipt.amount = 432.00;
        // expectedReceipt.dateTime = "2020-07-27T17:47:00";
        // expectedReceipt.fiscalDriveNumber = "9284000100287274";
        // expectedReceipt.fiscalDocumentNumber = "28889";
        // expectedReceipt.fiscalSign = "3906849540";

        // Настраиваем ожидаемое поведение мока
        EXPECT_CALL(*mockTransaction_, insert)
         .WillOnce([&request](const std::string & tableName, const std::vector< std::string > & cols, const std::vector< std::string > & data) {
             EXPECT_EQ(tableName, "receipts");
             EXPECT_EQ(cols, std::vector< std::string >{ "qrdata" });
             EXPECT_EQ(data, std::vector< std::string >{ request.qr_code_content() });
             return true;
         });

        // Act
        grpc::Status status = service_->ProcessQRCode(&context, &request, &response);

        // Assert
        EXPECT_TRUE(status.ok());
        // EXPECT_EQ(response.amount(), 432.00);
        // EXPECT_EQ(response.date_time(), "2020-07-27T17:47:00");
        // EXPECT_EQ(response.fiscal_drive_number(), "9284000100287274");
        // EXPECT_EQ(response.fiscal_document_number(), "28889");
        // EXPECT_EQ(response.fiscal_sign(), "3906849540");
    }

} // unnamed namespace
