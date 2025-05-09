LIBRARY(database_sqlite)

SRCS(
  ${PROJECT_SOURCE_DIR}/utils/database/sqlite/sqlite_database.h
  ${PROJECT_SOURCE_DIR}/utils/database/sqlite/sqlite_database.cpp
  ${PROJECT_SOURCE_DIR}/utils/database/sqlite/sqlite_transaction.h
  ${PROJECT_SOURCE_DIR}/utils/database/sqlite/sqlite_transaction.cpp
)

LIBS(
  database_interface
  database_transaction_base
  unofficial::sqlite3::sqlite3
  spdlog::spdlog
)

END()
