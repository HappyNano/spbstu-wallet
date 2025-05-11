#include <backend/service/service.h>
#include <utils/database/postgres/psql_database.h>

#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>

using grpc::Server;
using grpc::ServerBuilder;

void runServer() {
    const std::string serverAddress("0.0.0.0:50051");

    SPDLOG_INFO("Run server");

    auto db = std::make_unique< cxx::PsqlDatabase >();
    db->connect(cxx::PsqlDatabase::ConnectionInfo{
     .dbname = "wallet",
     .user = "admin",
     .password = "adminadmin",
     .host = "10.129.0.5",
     .port = "5432",
    });

    wallet::FinanceServiceImpl service(std::move(db));

    ServerBuilder builder;
    builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr< Server > server(builder.BuildAndStart());
    SPDLOG_INFO("Server listening on {}", serverAddress);

    server->Wait();
}

int main(int, char **) {
    // Init logger
    auto logger = spdlog::stdout_logger_mt("sdlmain");
#ifndef NDEBUG
    logger->set_level(spdlog::level::debug);
#else
    logger->set_level(spdlog::level::info);
#endif
    spdlog::set_default_logger(std::move(logger));

    // Отключаем xDS клиент
    setenv("GRPC_XDS_BOOTSTRAP", "{}", 1);
    runServer();
    return 0;
}
