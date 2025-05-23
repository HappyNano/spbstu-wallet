#pragma once

#include <proto/wallet/receipt/item.pb.h>

#include <nlohmann/json_fwd.hpp>

using namespace nlohmann;

namespace wallet {
    auto parseItemFromJson(const json & data) -> ReceiptItem;
} // namespace wallet
