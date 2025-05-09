GTEST("backend_service")

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/service/tests/service_test.cpp
)

LIBS(
  backend_service
  database_mock
)

END()
