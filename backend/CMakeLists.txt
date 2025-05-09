EXECUTABLE(backend_main)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/main.cpp
)

LIBS(
  backend_service
  database_postgres
  spdlog::spdlog
)

END()

add_subdirectory(receipt)
add_subdirectory(service)
