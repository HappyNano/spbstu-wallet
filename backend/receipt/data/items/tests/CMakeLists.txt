GTEST("backend_receipt_data_items")

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/receipt/data/items/tests/items_test.cpp
)

LIBS(
  backend_receipt_data_items
  spdlog::spdlog
)

END()
