#include "items.h"

#include <stdexcept>
#include <utils/string/doublespaces/doublespaces.h>
#include <utils/string/trim.h>

#include <nlohmann/json.hpp>

namespace {

    receipt::ReceiptItem::ENDSType getNDSType(int ndsInt) {
        using namespace receipt;
        return ReceiptItem::ENDSType_IsValid(ndsInt)
              ? static_cast< ReceiptItem::ENDSType >(ndsInt)
              : ReceiptItem::NDS_UNKNOWN;
    }

    receipt::ReceiptItem::EPaymentType getPaymentType(int paymentTypeInt) {
        using namespace receipt;
        return ReceiptItem::EPaymentType_IsValid(paymentTypeInt)
              ? static_cast< ReceiptItem::EPaymentType >(paymentTypeInt)
              : ReceiptItem::PAYMENT_TYPE_UNKNOWN;
    }

    receipt::ReceiptItem::EProductType getProductType(int productTypeInt) {
        using namespace receipt;
        return ReceiptItem::EProductType_IsValid(productTypeInt)
              ? static_cast< ReceiptItem::EProductType >(productTypeInt)
              : ReceiptItem::PRODUCT_TYPE_UNKNOWN;
    }

    receipt::ReceiptItem::EMeasurementUnit getMeasurementUnit(int measurementUnitInt) {
        using namespace receipt;
        return ReceiptItem::EMeasurementUnit_IsValid(measurementUnitInt)
              ? static_cast< ReceiptItem::EMeasurementUnit >(measurementUnitInt)
              : ReceiptItem::MEASUREMENT_UNIT_PIECE; // piece by default
    }

    std::string convertString(std::string s) {
        cxx::trim(s);
        std::replace(s.begin(), s.end(), '\t', ' ');
        cxx::removeDoubleSpaces(s);
        return s;
    }

} // unnamed namespace

auto receipt::parseItemFromJson(const json & data) -> ReceiptItem {
    if (!data.is_object()) {
        throw std::runtime_error("Bad json input");
    }

    ReceiptItem item;

    item.set_name(convertString(data["name"].get< std::string >()));
    item.set_price(data["price"].get< double >());
    item.set_quantity(data["quantity"].get< double >());
    item.set_sum(data["sum"].get< double >());
    item.set_nds_type(getNDSType(data["nds"].get< int >()));
    item.set_payment_type(getPaymentType(data["paymentType"].get< int >()));
    item.set_product_type(getProductType(data["productType"].get< int >()));
    item.set_measurement_unit(getMeasurementUnit(data["itemsQuantityMeasure"].get< int >()));

    return item;
}
