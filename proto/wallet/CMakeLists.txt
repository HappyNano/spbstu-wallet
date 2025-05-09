PROTO(GRPC)

SRCS(
  ${PROJECT_SOURCE_DIR}/proto/wallet/service.proto
)

LIBS(
  lib_proto_wallet_receipt_item
  lib_proto_wallet_receipt_receipt
)

END()

add_subdirectory(receipt)
