LIBRARY(database_sqlite)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/database/sqlite/sqlite_database.h
  ${PROJECT_SOURCE_DIR}/backend/database/sqlite/sqlite_database.cpp
)

LIBS(
  database_interface
  spdlog::spdlog
)

END()
