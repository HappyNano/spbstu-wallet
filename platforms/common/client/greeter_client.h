#pragma once

#include <memory>
#include <string>
#include <vector>

#include <platforms/common/client/interface/i_greeter_client.h>
#include <grpcpp/grpcpp.h>
#include <proto/database/database.grpc.pb.h>

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
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class DatabaseClient: public IDatabaseClient {
public:
    DatabaseClient(const std::shared_ptr< Channel > & channel);
    ~DatabaseClient() override = default;

    bool CreateTable(const std::string & table_name, const std::vector< std::pair< std::string, std::string > > & columns, bool primary_key_first = true) override;
    bool DropTable(const std::string & table_name) override;
    std::string SelectData(const std::string & table_name, const std::vector< std::string > & column_names = {}) override;
    bool InsertData(const std::string & table_name, const std::vector< std::string > & column_names, const std::vector< std::string > & values) override;
    bool UpdateData(const std::string & table_name, const std::vector< std::pair< std::string, std::string > > & column_values, const std::string & where_condition = "") override;
    bool DeleteData(const std::string & table_name, const std::string & where_condition = "") override;

private:
    std::unique_ptr< DatabaseService::Stub > stub_;
};
