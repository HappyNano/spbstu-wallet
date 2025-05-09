PROTO()
SRCS(${PROJECT_SOURCE_DIR}/proto/wallet/receipt/item.proto)
END()

PROTO()
SRCS(${PROJECT_SOURCE_DIR}/proto/wallet/receipt/receipt.proto)
LIBS(lib_proto_wallet_receipt_item)
END()
