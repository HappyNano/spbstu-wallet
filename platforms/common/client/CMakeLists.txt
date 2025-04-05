LIBRARY(common-client)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/common/client/greeter_client.cpp
  ${PROJECT_SOURCE_DIR}/platforms/common/client/greeter_client.h
)

LIBS(
  lib_proto_helloworld_helloworld
  common-client_interface
)

END()

add_subdirectory(interface)
