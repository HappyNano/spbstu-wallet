#include <iostream>
#include <memory>
#include <string>

#define GRPC_XDS_BOOTSTRAP_CONFIG "{}"
#include <grpcpp/grpcpp.h>
#include <proto/database/database.grpc.pb.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>

#include <backend/database/postgres/psql_database.h>

using databaseservice::Column;
using databaseservice::CreateTableRequest;
using databaseservice::DatabaseService;
using databaseservice::DeleteRequest;
using databaseservice::DropTableRequest;
using databaseservice::InsertRequest;
using databaseservice::Row;
using databaseservice::SelectRequest;
using databaseservice::SelectResponse;
using databaseservice::StatusResponse;
using databaseservice::UpdateRequest;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class DatabaseServiceImpl final: public DatabaseService::Service {
public:
    DatabaseServiceImpl() {
        db_ = std::make_unique< cxx::PsqlDatabase >();
        db_->connect("dbname=wallet user=admin password=adminadmin host=10.129.0.5 port=5432");
    }

    Status CreateTable(ServerContext * context, const CreateTableRequest * request, StatusResponse * response) override {
        std::vector< cxx::Col > columns;

        // Преобразуем protobuf колонки в наш тип Col
        for (const auto & col: request->columns()) {
            cxx::Col column;
            column.name = col.name();

            // Преобразуем тип данных
            switch (col.type()) {
            case Column::INTEGER:
                column.type = cxx::Col::EDataType::INTEGER;
                break;
            case Column::REAL:
                column.type = cxx::Col::EDataType::REAL;
                break;
            case Column::TEXT:
                column.type = cxx::Col::EDataType::TEXT;
                break;
            case Column::BOOLEAN:
                column.type = cxx::Col::EDataType::BOOLEAN;
                break;
            case Column::DATE:
                column.type = cxx::Col::EDataType::DATE;
                break;
            case Column::TIMESTAMP:
                column.type = cxx::Col::EDataType::TIMESTAMP;
                break;
            default:
                column.type = cxx::Col::EDataType::TEXT;
                break;
            }

            // Преобразуем ограничения
            switch (col.constraint()) {
            case Column::PRIMARY_KEY:
                column.constraint = cxx::Col::EConstraint::PRIMARY_KEY;
                break;
            case Column::UNIQUE:
                column.constraint = cxx::Col::EConstraint::UNIQUE;
                break;
            case Column::NOT_NULL:
                column.constraint = cxx::Col::EConstraint::NOT_NULL;
                break;
            case Column::FOREIGN_KEY:
                column.constraint = cxx::Col::EConstraint::FOREIGN_KEY;
                break;
            default:
                column.constraint = cxx::Col::EConstraint::NONE;
                break;
            }

            columns.push_back(column);
        }

        bool success = db_->createTable(request->table_name(), columns);

        response->set_success(success);
        if (success) {
            response->set_message("Table created successfully");
        } else {
            response->set_message("Failed to create table");
        }

        return Status::OK;
    }

    Status DropTable(ServerContext * context, const DropTableRequest * request, StatusResponse * response) override {
        bool success = db_->dropTable(request->table_name());

        response->set_success(success);
        if (success) {
            response->set_message("Table dropped successfully");
        } else {
            response->set_message("Failed to drop table");
        }

        return Status::OK;
    }

    Status SelectData(ServerContext * context, const SelectRequest * request, SelectResponse * response) override {
        std::vector< std::string > columns;
        for (const auto & col: request->column_names()) {
            columns.push_back(col);
        }

        auto result = db_->select(request->table_name(), columns);

        if (result) {
            response->set_success(true);

            // Преобразуем результаты в протобуферы
            for (const auto & row_data: *result) {
                Row * row = response->add_rows();

                for (const auto & cell: row_data) {
                    if (std::holds_alternative< std::string >(cell)) {
                        row->add_values(std::get< std::string >(cell));
                    } else if (std::holds_alternative< int >(cell)) {
                        row->add_values(std::to_string(std::get< int >(cell)));
                    } else if (std::holds_alternative< double >(cell)) {
                        row->add_values(std::to_string(std::get< double >(cell)));
                    } else if (std::holds_alternative< bool >(cell)) {
                        row->add_values(std::get< bool >(cell) ? "true" : "false");
                    }
                }
            }
        } else {
            response->set_success(false);
            response->set_error_message("Failed to execute select query");
        }

        return Status::OK;
    }

    Status InsertData(ServerContext * context, const InsertRequest * request, StatusResponse * response) override {
        std::vector< std::string > columns;
        for (const auto & col: request->column_names()) {
            columns.push_back(col);
        }

        std::vector< std::string > values;
        for (const auto & val: request->values()) {
            values.push_back(val);
        }

        bool success = db_->insert(request->table_name(), columns, values);

        response->set_success(success);
        if (success) {
            response->set_message("Data inserted successfully");
        } else {
            response->set_message("Failed to insert data");
        }

        return Status::OK;
    }

    Status UpdateData(ServerContext * context, const UpdateRequest * request, StatusResponse * response) override {
        std::vector< std::pair< std::string, std::string > > columnValuePairs;

        for (const auto & col_val: request->column_values()) {
            columnValuePairs.emplace_back(col_val.column_name(), col_val.value());
        }

        bool success = db_->update(request->table_name(), columnValuePairs, request->where_condition());

        response->set_success(success);
        if (success) {
            response->set_message("Data updated successfully");
        } else {
            response->set_message("Failed to update data");
        }

        return Status::OK;
    }

    Status DeleteData(ServerContext * context, const DeleteRequest * request, StatusResponse * response) override {
        bool success = db_->deleteFrom(request->table_name(), request->where_condition());

        response->set_success(success);
        if (success) {
            response->set_message("Data deleted successfully");
        } else {
            response->set_message("Failed to delete data");
        }

        return Status::OK;
    }

private:
    std::unique_ptr< cxx::IDatabase > db_;
};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    SPDLOG_INFO("Initializing service");
    DatabaseServiceImpl service;

    SPDLOG_INFO("Building server with max message size unlimited");
    ServerBuilder builder;

    // Установите параметры сервера
    builder.SetMaxReceiveMessageSize(INT_MAX);
    builder.SetMaxSendMessageSize(INT_MAX);
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    SPDLOG_INFO("Starting server");
    std::unique_ptr<Server> server = builder.BuildAndStart();
    if (!server) {
        SPDLOG_ERROR("Failed to start server");
        return;
    }

    std::cout << "Server listening on " << server_address << std::endl;
    SPDLOG_INFO("Server listening on {}", server_address);

    // Блокирующий вызов до завершения сервера
    server->Wait();
}


int main(int argc, char ** argv) {
    SPDLOG_INFO("Here1");
    // Init logger
    auto logger = spdlog::stdout_logger_mt("sdlmain");
    #ifndef NDEBUG
    logger->set_level(spdlog::level::debug);
    #else
    logger->set_level(spdlog::level::info);
    #endif
    spdlog::set_default_logger(std::move(logger));
    SPDLOG_INFO("Here2");

    // Отключаем xDS клиент
    setenv("GRPC_XDS_BOOTSTRAP", "{}", 1);
    SPDLOG_INFO("Here3");
    RunServer();
    return 0;
}
