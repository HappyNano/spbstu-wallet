LIBRARY(backend_receipt_ofd_http)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/receipt/ofd/http/http_ofd.h
  ${PROJECT_SOURCE_DIR}/backend/receipt/ofd/http/http_ofd.cpp
)

LIBS(
  backend_receipt_ofd_interface
  backend_receipt_data_items
  utils_string
  spdlog::spdlog
)

END()

option(TEST_OFD "" OFF)
if(TEST_OFD)
  ADD_TESTS(tests)
endif()
