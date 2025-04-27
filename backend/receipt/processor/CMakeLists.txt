LIBRARY(backend_receipt_processor)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/receipt/processor/receipt_processor.h
  ${PROJECT_SOURCE_DIR}/backend/receipt/processor/receipt_processor.cpp
)

LIBS(
  lib_proto_wallet_service
)

END()
