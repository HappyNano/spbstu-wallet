LIBRARY(database_interface)

SRCS(
  ${PROJECT_SOURCE_DIR}/utils/database/interface/i_database.h
  ${PROJECT_SOURCE_DIR}/utils/database/interface/i_database.cpp
)

LIBS(
  database_transaction_interface
)

END()
