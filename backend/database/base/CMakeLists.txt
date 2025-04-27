LIBRARY(database_base)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/database/base/base_database.h
  ${PROJECT_SOURCE_DIR}/backend/database/base/base_database.cpp
)

LIBS(
  database_interface
)

END()
