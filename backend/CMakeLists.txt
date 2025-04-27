EXECUTABLE(backend_main)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/main.cpp
)

LIBS(
  lib_proto_wallet_service
  backend_service
  database_postgres
  spdlog::spdlog
)

END()

add_subdirectory(database)
add_subdirectory(receipt)
add_subdirectory(service)
