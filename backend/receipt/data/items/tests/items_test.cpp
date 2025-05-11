#include <backend/receipt/data/items/items.h>

#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include <nlohmann/json.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>

using namespace wallet;
using namespace testing;
using namespace nlohmann;

namespace {

    const nlohmann::json ITEM_JSON = json::parse(R"(
{
    "nds": 1,
    "sum": 26699,
    "name": "Тестовый образец",
    "price": 26699,
    "quantity": 1,
    "paymentType": 4,
    "productType": 1,
    "itemsQuantityMeasure": 0
})");

    class HttpOFDTest: public ::Test {
    public:
        void SetUp() override {
            auto sink = std::make_shared< spdlog::sinks::stdout_color_sink_mt >();
            auto logger = spdlog::stdout_logger_mt("gtest_logger");
            logger->set_level(spdlog::level::debug);
            spdlog::set_default_logger(std::move(logger));

            testReceiptItem_.set_name("Тестовый образец");
            testReceiptItem_.set_price(26699.0);
            testReceiptItem_.set_quantity(1);
            testReceiptItem_.set_sum(26699.0);
            testReceiptItem_.set_nds_type(wallet::ReceiptItem_ENDSType_NDS_20);
            testReceiptItem_.set_payment_type(wallet::ReceiptItem_EPaymentType_PAYMENT_TYPE_FULL_PAYMENT);
            testReceiptItem_.set_product_type(wallet::ReceiptItem_EProductType_PRODUCT_TYPE_GOODS);
            testReceiptItem_.set_measurement_unit(wallet::ReceiptItem_EMeasurementUnit_MEASUREMENT_UNIT_PIECE);
        }

    protected:
        ReceiptItem testReceiptItem_;
    };

} // unnamed namespace

TEST_F(HttpOFDTest, DefaultReceipt) {
    // Arrange
    ReceiptItem receiptItem = parseItemFromJson(ITEM_JSON);
    EXPECT_TRUE(google::protobuf::util::MessageDifferencer::Equals(receiptItem, testReceiptItem_));
}
