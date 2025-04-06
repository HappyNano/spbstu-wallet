#pragma once

#include <string>

class IDatabaseClient {
public:
    virtual ~IDatabaseClient() = default;

    virtual bool CreateTable(const std::string & table_name, const std::vector< std::pair< std::string, std::string > > & columns, bool primary_key_first = true) = 0;
    virtual bool DropTable(const std::string & table_name) = 0;
    virtual std::string SelectData(const std::string & table_name, const std::vector< std::string > & column_names = {}) = 0;
    virtual bool InsertData(const std::string & table_name, const std::vector< std::string > & column_names, const std::vector< std::string > & values) = 0;
    virtual bool UpdateData(const std::string & table_name, const std::vector< std::pair< std::string, std::string > > & column_values, const std::string & where_condition = "") = 0;
    virtual bool DeleteData(const std::string & table_name, const std::string & where_condition = "") = 0;
};
