#pragma once

#include <pqxx/pqxx>

#include <backend/database/interface/i_database.h>

#include <memory>
#include <string>

namespace cxx {

    class PsqlDatabase final: public IDatabase {
    public:
        struct ConnectionInfo {
            std::string dbname;
            std::string user;
            std::string password;
            std::string host;
            std::string port;

            std::string toString() const;
        };

    public:
        PsqlDatabase();
        ~PsqlDatabase() override;

        bool connect(const ConnectionInfo & connectionInfo);
        bool connect(const std::string & connectionString) override;
        void disconnect() override;
        bool createTable(const std::string & name, const std::vector< Col > & cols) override;
        bool dropTable(const std::string & tableName) override;
        std::optional< QueryResult > select(const std::string & fromTableName, const std::vector< std::string > & colsName) override;
        bool insert(const std::string & tableName, const std::vector< std::string > & colNames, const std::vector< std::string > & values) override;
        bool update(const std::string & tableName, const std::vector< std::pair< std::string, std::string > > & colValuePairs, const std::string & whereCondition) override;
        bool deleteFrom(const std::string & tableName, const std::string & whereCondition) override;
        std::optional< QueryResult > executeQuery(const std::string & query) override;

    private:
        std::unique_ptr< pqxx::connection > conn_;
        bool isConnected_;

        static std::string escapeString(const std::string & str);
    };

} // namespace cxx
