EXECUTABLE(backend_main)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/main.cpp
)

LIBS(
  lib_proto_helloworld_helloworld
)

END()
