LIBRARY(backend_service)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/service/service.h
  ${PROJECT_SOURCE_DIR}/backend/service/service.cpp
)

LIBS(
  lib_proto_wallet_service
  backend_receipt_data_qr

  database_postgres
  spdlog::spdlog
)

END()

ADD_TESTS(tests)
