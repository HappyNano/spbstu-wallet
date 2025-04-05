EXECUTABLE(backend_main)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/main.cpp
)

PROTOS(
  proto_test
  proto_testdir_pp
)

END()
