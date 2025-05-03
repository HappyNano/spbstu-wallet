#pragma once

#include <utils/database/interface/i_database.h>
#include <utils/database/transaction/interface/i_transaction.h>

#include <pqxx/pqxx>

#include <memory>
#include <optional>
#include <string>

namespace cxx {

    class PsqlDatabase final: public IDatabase {
    public:
        struct ConnectionInfo {
            std::optional< std::string > dbname;
            std::optional< std::string > user;
            std::optional< std::string > password;
            std::optional< std::string > host;
            std::optional< std::string > port;

            std::string toString() const;
        };

    public:
        PsqlDatabase();
        ~PsqlDatabase() override;

        bool connect(const ConnectionInfo & connectionInfo);
        bool connect(const std::string & connectionString);
        void disconnect();

        // IDatabase
        std::unique_ptr< ITransaction > makeTransaction() override;

    private:
        std::unique_ptr< pqxx::connection > conn_;
    };

} // namespace cxx
