GTEST("backend_receipt_ofd_http")

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/receipt/ofd/http/tests/ofd_test.cpp
)

LIBS(
  backend_receipt_ofd_http
  backend_receipt_data_items
  http_client_curl
  spdlog::spdlog
)

END()
