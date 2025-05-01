LIBRARY(database_mock INTERFACE)

SRCS(
  ${PROJECT_SOURCE_DIR}/utils/database/mock/mock_database.h
)

LIBS(
  GTest::gmock
)

END()
