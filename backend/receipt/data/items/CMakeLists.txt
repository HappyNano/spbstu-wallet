LIBRARY(backend_receipt_data_items)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/receipt/data/items/items.h
  ${PROJECT_SOURCE_DIR}/backend/receipt/data/items/items.cpp
)

LIBS(
  lib_proto_wallet_receipt_item
  utils_string
  utils_string_doublespaces
  nlohmann_json::nlohmann_json
)

END()

ADD_TESTS(tests)
