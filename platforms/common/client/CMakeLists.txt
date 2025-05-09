LIBRARY(common-client)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/common/client/greeter_client.cpp
  ${PROJECT_SOURCE_DIR}/platforms/common/client/greeter_client.h
)

LIBS(
  common-client_interface
  lib_proto_wallet_service
  spdlog::spdlog
)

END()

add_subdirectory(interface)
