GTEST("database_sqlite_test")

SRCS(
  ${PROJECT_SOURCE_DIR}/utils/database/sqlite/tests/common_test.cpp
)

LIBS(
  database_sqlite
)

END()
