EXECUTABLE(backend_main)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/main.cpp
)

LIBS(
  lib_proto_helloworld_helloworld
  lib_proto_database_database
  database_postgres
)

END()

add_subdirectory(database)
