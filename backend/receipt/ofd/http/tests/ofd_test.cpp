// NEED TO TEST_OFD=ON

#include <backend/receipt/data/items/items.h>
#include <backend/receipt/ofd/http/http_ofd.h>
#include <utils/http/client/curl/curl_http_client.h>

#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include <nlohmann/json.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <memory>

using namespace cxx;
using namespace receipt;
using namespace testing;
using namespace nlohmann;

namespace {

    const nlohmann::json ITEMS_JSON = json::parse(R"(
[{
    "nds": 1,
    "sum": 26699,
    "name": "PAP.Бум.туал.3сл белая 12рул",
    "price": 26699,
    "quantity": 1,
    "paymentType": 4,
    "productType": 1,
    "itemsQuantityMeasure": 0
},
{
    "nds": 1,
    "sum": 999,
    "name": "Пакет ПЯТЕРОЧКА 65х40см",
    "price": 999,
    "quantity": 1,
    "paymentType": 4,
    "productType": 1,
    "itemsQuantityMeasure": 0
},
{
    "nds": 2,
    "sum": 12999,
    "name": "GL.VIL.Нектар мультифрукт 0,95л",
    "price": 12999,
    "quantity": 1,
    "paymentType": 4,
    "productType": 33,
    "itemsQuantityMeasure": 0
},
{
    "nds": 2,
    "sum": 10699,
    "name": "САРАФ.Мол.паст.дет.2,5% 930мл",
    "price": 10699,
    "quantity": 1,
    "paymentType": 4,
    "productType": 33,
    "itemsQuantityMeasure": 0
},
{
    "nds": 1,
    "sum": 47994,
    "name": "PICNIC Батончик BIG 76г",
    "price": 7999,
    "quantity": 6,
    "paymentType": 4,
    "productType": 1,
    "itemsQuantityMeasure": 0
},
{
    "nds": 1,
    "sum": 9999,
    "name": "GL.VIL.S.Кукуруза сладкая 340г",
    "price": 9999,
    "quantity": 1,
    "paymentType": 4,
    "productType": 1,
    "itemsQuantityMeasure": 0
},
{
    "nds": 1,
    "sum": 9899,
    "name": "GLOBUS Кукуруза сладк.ж\/б 340г",
    "price": 9899,
    "quantity": 1,
    "paymentType": 4,
    "productType": 1,
    "itemsQuantityMeasure": 0
},
{
    "nds": 2,
    "sum": 27998,
    "name": "ПАПА МОЖ.Колб.СЕРВ.ФИН.в\/к 350г",
    "price": 13999,
    "quantity": 2,
    "paymentType": 4,
    "productType": 1,
    "itemsQuantityMeasure": 0
},
{
    "nds": 2,
    "sum": 11398,
    "name": "Брецель-дог с кунжутом 159г",
    "price": 5699,
    "quantity": 2,
    "paymentType": 4,
    "productType": 1,
    "itemsQuantityMeasure": 0
}])");

    class HttpOFDTest: public ::Test {
    public:
        void SetUp() override {
            fillReceiptData();

            auto sink = std::make_shared< spdlog::sinks::stdout_color_sink_mt >();
            auto logger = spdlog::stdout_logger_mt("gtest_logger");
            logger->set_level(spdlog::level::debug);
            spdlog::set_default_logger(std::move(logger));

            auto client = std::make_shared< CurlHttpClient >(CurlHttpClient::Settings{}, IHttpClient::Headers{});
            HttpOFD::Settings ofdSettings{
                .token = std::getenv("OFD_TOKEN"),
            };
            ofd_ = std::make_unique< HttpOFD >(client, ofdSettings);
        }

        void fillReceiptData() {
            // docs/ofd_out.json
            testReceipt_.set_t("20200727T174700");
            testReceipt_.set_s(1586.84);
            testReceipt_.set_fn(7282440500213781);
            testReceipt_.set_i(65152);
            testReceipt_.set_fp(615715057);
            testReceipt_.set_n(1);

            testReceiptData_.mutable_receipt()->CopyFrom(testReceipt_);
            for (const auto & itemJson: ITEMS_JSON) {
                ReceiptItem item = parseItemFromJson(itemJson);
                testReceiptData_.add_items()->Swap(&item);
            }
            auto * retailerInfo = testReceiptData_.mutable_retailer();
            retailerInfo->set_name("ООО \"Агроторг\"");
            retailerInfo->set_place("Q539 7050-Пятерочка");
            retailerInfo->set_inn("7825706086");
            retailerInfo->set_address("194100, 78, город федерального значения Санкт-Петербург, Александра Матросова ул, 20, 2 лит.А, пом.20-Н");
        }

    protected:
        std::unique_ptr< HttpOFD > ofd_;
        Receipt testReceipt_;
        ReceiptData testReceiptData_;
    };

} // unnamed namespace

TEST_F(HttpOFDTest, Defaults) {
    // Arrange
    auto receiptData = ofd_->getReceiptData(testReceipt_);
    EXPECT_TRUE(google::protobuf::util::MessageDifferencer::Equals(receiptData, testReceiptData_));
}
