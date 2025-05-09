LIBRARY(backend_receipt_database)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/receipt/database/receipt_database.h
  ${PROJECT_SOURCE_DIR}/backend/receipt/database/receipt_database.cpp
)

LIBS(
  lib_proto_wallet_receipt_receipt
  database_interface
)

END()
