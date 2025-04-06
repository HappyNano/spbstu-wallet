LIBRARY(database_postgres)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/database/postgres/psql_database.h
  ${PROJECT_SOURCE_DIR}/backend/database/postgres/psql_database.cpp
)

LIBS(
  database_interface
  libpqxx::pqxx
  spdlog::spdlog
)

END()
