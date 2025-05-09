LIBRARY(backend_receipt_ofd_mock INTERFACE)

SRCS(
  ${PROJECT_SOURCE_DIR}/backend/receipt/ofd/mock/mock_ofd.h
)

LIBS(
  backend_receipt_ofd_interface
)

END()
