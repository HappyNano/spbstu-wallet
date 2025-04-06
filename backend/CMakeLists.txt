EXECUTABLE(backend_main)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/main.cpp
)

LIBS(
  lib_proto_database_database
  database_postgres
  spdlog::spdlog
)

END()

add_subdirectory(database)
