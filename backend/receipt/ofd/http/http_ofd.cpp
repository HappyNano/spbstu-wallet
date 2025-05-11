#include "http_ofd.h"

#include <backend/receipt/data/items/items.h>

#include <utils/string/trim.h>

#include <nlohmann/json.hpp>
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include <spdlog/spdlog.h>

#include <sstream>
#include <stdexcept>

using namespace cxx;
using namespace wallet;
using namespace nlohmann;

namespace {

    constexpr const char * OFD_API_URL = "https://proverkacheka.com/api/v1/check/get";

} // unnamed namespace

HttpOFD::HttpOFD(std::shared_ptr< cxx::IHttpClient > httpClient, Settings settings)
  : settings_{ std::move(settings) }
  , httpClient_{ std::move(httpClient) } {
}

auto HttpOFD::getReceiptData(const Receipt & receipt) -> ReceiptData {
    SPDLOG_DEBUG("HttpOFD::getReceiptData");

    const auto response = httpClient_->post(OFD_API_URL, makeRequestBody(receipt));
    if (response.statusCode != 200) {
        throw std::runtime_error("HttpOFD: Bad request");
    }

    SPDLOG_DEBUG("New data from OFD {}", response.body);

    auto responseJson = json::parse(response.body);
    const auto & data = responseJson["data"];

    ReceiptData receiptData;
    receiptData.mutable_receipt()->CopyFrom(receipt);
    if (!data.contains("json")) {
        if (data.is_string()) {
            SPDLOG_ERROR("Failed to get receipt data from OFD. Message: {}", data.dump());
        } else {
            SPDLOG_ERROR("Failed to get receipt data from OFD. Data is not string");
        }
        return receiptData;
    }

    const auto & jsonData = data["json"];

    for (const auto & itemJson: jsonData["items"]) {
        ReceiptItem item = parseItemFromJson(itemJson);
        receiptData.add_items()->Swap(&item);
    }

    auto * retailerInfo = receiptData.mutable_retailer();
    retailerInfo->set_name(cxx::trimCopy(jsonData["user"].get< std::string >()));
    retailerInfo->set_place(cxx::trimCopy(jsonData["retailPlace"].get< std::string >()));
    retailerInfo->set_inn(cxx::trimCopy(jsonData["userInn"].get< std::string >()));
    retailerInfo->set_address(cxx::trimCopy(jsonData["retailPlaceAddress"].get< std::string >()));

    return receiptData;
}

auto HttpOFD::makeRequestBody(const Receipt & receipt) const -> std::string {
    std::stringstream ss;

    ss << "token=" << settings_.token;
    ss << "&fn=" << receipt.fn();
    ss << "&fd=" << receipt.i();
    ss << "&fp=" << receipt.fp();
    ss << "&t=" << receipt.t();
    ss << "&n=" << receipt.n();
    ss << "&s=" << receipt.s();
    ss << "&qr=1";

    return ss.str();
}
