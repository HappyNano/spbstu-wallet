#pragma once

#include <pqxx/pqxx>

#include <utils/database/base/base_database.h>

#include <memory>
#include <string>

namespace cxx {

    class PsqlDatabase final: public BaseDatabase {
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
        std::optional< QueryResult > executeQuery(const std::string & query) override;

    private:
        bool isReady() const noexcept override;
        std::string escapeString(const std::string & str) override;

    private:
        std::unique_ptr< pqxx::connection > conn_;
    };

} // namespace cxx
