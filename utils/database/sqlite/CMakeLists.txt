LIBRARY(database_sqlite)

SRCS(
  ${PROJECT_SOURCE_DIR}/utils/database/sqlite/sqlite_database.h
  ${PROJECT_SOURCE_DIR}/utils/database/sqlite/sqlite_database.cpp
)

LIBS(
  database_base
  unofficial::sqlite3::sqlite3
  spdlog::spdlog
)

END()

ADD_TESTS(tests)
