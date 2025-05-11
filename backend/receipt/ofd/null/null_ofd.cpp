#include "null_ofd.h"
#include "proto/wallet/receipt/receipt.pb.h"

using namespace wallet;

auto NullOFD::getReceiptData(const Receipt & receipt) -> ReceiptData {
    ReceiptData receiptData;
    receiptData.mutable_receipt()->CopyFrom(receipt);
    return receiptData;
}
