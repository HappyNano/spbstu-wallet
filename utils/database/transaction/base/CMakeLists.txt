LIBRARY(database_transaction_base)

SRCS(
  ${PROJECT_SOURCE_DIR}/utils/database/transaction/base/base_transaction.h
  ${PROJECT_SOURCE_DIR}/utils/database/transaction/base/base_transaction.cpp
)

LIBS(
  database_transaction_interface
)

END()
