LIBRARY(common-client)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/common/client/finance_client.cpp
  ${PROJECT_SOURCE_DIR}/platforms/common/client/finance_client.h
)

LIBS(
  lib_proto_wallet_service
  spdlog::spdlog
)

END()
