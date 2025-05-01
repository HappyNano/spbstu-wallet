LIBRARY(database_postgres)

SRCS(
  ${PROJECT_SOURCE_DIR}/utils/database/postgres/psql_database.h
  ${PROJECT_SOURCE_DIR}/utils/database/postgres/psql_database.cpp
)

LIBS(
  database_base
  libpqxx::pqxx
  spdlog::spdlog
)

END()
