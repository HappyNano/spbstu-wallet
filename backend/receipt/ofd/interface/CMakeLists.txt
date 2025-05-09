LIBRARY(backend_receipt_ofd_interface)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/receipt/ofd/interface/i_ofd.h
  ${PROJECT_SOURCE_DIR}/backend/receipt/ofd/interface/i_ofd.cpp
)

LIBS(
  lib_proto_wallet_receipt_receipt
)

END()
