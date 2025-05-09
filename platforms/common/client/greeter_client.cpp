#include "greeter_client.h"
#include <grpcpp/grpcpp.h>

#include <spdlog/spdlog.h>

ReceiptScannerClient::ReceiptScannerClient(const std::shared_ptr< Channel > & channel)
  : stub_(ReceiptScannerService::NewStub(channel)) {
}

ReceiptScannerClient::Response ReceiptScannerClient::ProcessQRCode(const std::string & user_id, const std::string & qr_code) {
    QRCodeRequest request;
    request.set_user_id(user_id);
    request.set_qr_code_content(qr_code);
    request.set_timestamp(
     std::chrono::duration_cast< std::chrono::seconds >(
      std::chrono::system_clock::now().time_since_epoch())
      .count());

    ReceiptResponse response;
    ClientContext context;

    // Вызываем удаленный метод
    Status status = stub_->ProcessQRCode(&context, request, &response);

    // Проверяем статус ответа
    if (!status.ok()) {
        std::cerr << "RPC failed: " << status.error_message() << std::endl;
        return Response{ .error = "something wrong" };
    }

    Response result;
    // Обрабатываем результат
    if (response.has_receipt()) {
        result.fn = response.receipt().fn();
        result.fp = response.receipt().fp();
        result.i = response.receipt().i();
        result.n = response.receipt().n();
        result.s = response.receipt().s();
        result.t = response.receipt().t();
    } else {
        result.error = response.error().message();
    }
    return result;
}
