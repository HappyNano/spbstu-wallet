LIBRARY(database_postgres)

SRCS(
  ${PROJECT_SOURCE_DIR}/utils/database/postgres/psql_database.h
  ${PROJECT_SOURCE_DIR}/utils/database/postgres/psql_database.cpp
  ${PROJECT_SOURCE_DIR}/utils/database/postgres/psql_transaction.h
  ${PROJECT_SOURCE_DIR}/utils/database/postgres/psql_transaction.cpp
)

LIBS(
  utils_string

  database_base_transaction
  libpqxx::pqxx
  spdlog::spdlog
)

END()
