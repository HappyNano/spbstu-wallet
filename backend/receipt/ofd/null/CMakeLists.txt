LIBRARY(backend_receipt_ofd_null)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/receipt/ofd/null/null_ofd.h
  ${PROJECT_SOURCE_DIR}/backend/receipt/ofd/null/null_ofd.cpp
)

LIBS(
  backend_receipt_ofd_interface
)

END()
