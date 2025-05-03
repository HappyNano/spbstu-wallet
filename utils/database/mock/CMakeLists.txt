LIBRARY(database_mock INTERFACE)

SRCS(
  ${PROJECT_SOURCE_DIR}/utils/database/mock/mock_database.h
  ${PROJECT_SOURCE_DIR}/utils/database/mock/mock_transaction.h
)

LIBS(
  GTest::gmock
)

END()
