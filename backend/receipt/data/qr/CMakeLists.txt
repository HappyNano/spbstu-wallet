LIBRARY(backend_receipt_data_qr)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/receipt/data/qr/qr.h
  ${PROJECT_SOURCE_DIR}/backend/receipt/data/qr/qr.cpp
)

LIBS(
  lib_proto_wallet_receipt_receipt
)

END()
