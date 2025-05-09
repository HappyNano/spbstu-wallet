GTEST("databases_test")

SRCS(
  ${PROJECT_SOURCE_DIR}/utils/database/tests/common_test.cpp
)

LIBS(
  database_sqlite
  database_postgres
)

END()
