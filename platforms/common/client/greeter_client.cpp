#include "greeter_client.h"
#include <grpcpp/grpcpp.h>

#include <spdlog/spdlog.h>
#include <sstream>

DatabaseClient::DatabaseClient(const std::shared_ptr< Channel > & channel)
  : stub_(DatabaseService::NewStub(channel)) {
}

// Создание таблицы
bool DatabaseClient::CreateTable(const std::string & table_name, const std::vector< std::pair< std::string, std::string > > & columns, bool primary_key_first) {
    CreateTableRequest request;
    request.set_table_name(table_name);

    for (const auto & col_info: columns) {
        Column * column = request.add_columns();
        column->set_name(col_info.first);

        // Определяем тип колонки
        if (col_info.second == "INTEGER") {
            column->set_type(Column::INTEGER);
        } else if (col_info.second == "REAL") {
            column->set_type(Column::REAL);
        } else if (col_info.second == "BOOLEAN") {
            column->set_type(Column::BOOLEAN);
        } else if (col_info.second == "DATE") {
            column->set_type(Column::DATE);
        } else if (col_info.second == "TIMESTAMP") {
            column->set_type(Column::TIMESTAMP);
        } else {
            column->set_type(Column::TEXT); // По умолчанию TEXT
        }

        // Если это первая колонка и primary_key_first = true, делаем её первичным ключом
        if (primary_key_first && col_info == columns.front()) {
            column->set_constraint(Column::PRIMARY_KEY);
        } else {
            column->set_constraint(Column::NONE);
        }
    }

    StatusResponse response;
    ClientContext context;

    Status status = stub_->CreateTable(&context, request, &response);

    if (status.ok()) {
        std::cout << "Create table result: " << response.message() << std::endl;
        return response.success();
    } else {
        std::cerr << "RPC failed: " << status.error_message() << std::endl;
        return false;
    }
}

// Удаление таблицы
bool DatabaseClient::DropTable(const std::string & table_name) {
    DropTableRequest request;
    request.set_table_name(table_name);

    StatusResponse response;
    ClientContext context;

    Status status = stub_->DropTable(&context, request, &response);

    if (status.ok()) {
        std::cout << "Drop table result: " << response.message() << std::endl;
        return response.success();
    } else {
        std::cerr << "RPC failed: " << status.error_message() << std::endl;
        return false;
    }
}

// Выборка данных из таблицы
std::string DatabaseClient::SelectData(const std::string & table_name, const std::vector< std::string > & column_names) {
    SelectRequest request;
    request.set_table_name(table_name);

    for (const auto & col: column_names) {
        request.add_column_names(col);
    }

    SelectResponse response;
    ClientContext context;

    Status status = stub_->SelectData(&context, request, &response);

    if (status.ok()) {
        if (response.success()) {
            std::stringstream ss;
            ss << "Select data successful. Results:" << std::endl;

            // Вывод заголовков
            if (!column_names.empty()) {
                for (const auto & col: column_names) {
                    ss << col << "\t";
                }
            } else {
                ss << "(All columns)";
            }
            ss << std::endl;

            // Вывод данных
            for (const auto & row: response.rows()) {
                for (const auto & value: row.values()) {
                    ss << value << "\t";
                }
                ss << std::endl;
            }

            return ss.str();
        } else {
            std::cerr << "Select failed: " << response.error_message() << std::endl;
            return "err";
        }
    } else {
        std::cerr << "RPC failed: " << status.error_message() << std::endl;
        return "rpc err";
    }
}

// Вставка данных в таблицу
bool DatabaseClient::InsertData(const std::string & table_name, const std::vector< std::string > & column_names, const std::vector< std::string > & values) {
    if (column_names.size() != values.size()) {
        std::cerr << "Error: Number of columns doesn't match number of values" << std::endl;
        return false;
    }

    InsertRequest request;
    request.set_table_name(table_name);

    for (const auto & col: column_names) {
        request.add_column_names(col);
    }

    for (const auto & val: values) {
        request.add_values(val);
    }

    StatusResponse response;
    ClientContext context;

    Status status = stub_->InsertData(&context, request, &response);

    if (status.ok()) {
        std::cout << "Insert data result: " << response.message() << std::endl;
        return response.success();
    } else {
        std::cerr << "RPC failed: " << status.error_message() << std::endl;
        return false;
    }
}

// Обновление данных в таблице
bool DatabaseClient::UpdateData(const std::string & table_name, const std::vector< std::pair< std::string, std::string > > & column_values, const std::string & where_condition) {
    UpdateRequest request;
    request.set_table_name(table_name);
    request.set_where_condition(where_condition);

    for (const auto & col_val: column_values) {
        auto * column_value = request.add_column_values();
        column_value->set_column_name(col_val.first);
        column_value->set_value(col_val.second);
    }

    StatusResponse response;
    ClientContext context;

    Status status = stub_->UpdateData(&context, request, &response);

    if (status.ok()) {
        std::cout << "Update data result: " << response.message() << std::endl;
        return response.success();
    } else {
        std::cerr << "RPC failed: " << status.error_message() << std::endl;
        return false;
    }
}

// Удаление данных из таблицы
bool DatabaseClient::DeleteData(const std::string & table_name, const std::string & where_condition) {
    DeleteRequest request;
    request.set_table_name(table_name);
    request.set_where_condition(where_condition);

    StatusResponse response;
    ClientContext context;

    Status status = stub_->DeleteData(&context, request, &response);

    if (status.ok()) {
        std::cout << "Delete data result: " << response.message() << std::endl;
        return response.success();
    } else {
        std::cerr << "RPC failed: " << status.error_message() << std::endl;
        return false;
    }
}
