#pragma once

#include <utils/database/interface/i_database.h>
#include <utils/database/transaction/interface/i_transaction.h>

#include <pqxx/pqxx>

#include <memory>
#include <optional>
#include <string>

namespace cxx {

    /**
     * @brief PostgreSQL implementation of the IDatabase interface
     *
     * This class provides PostgreSQL-specific database functionality, including
     * connection management and transaction creation.
     */
    class PsqlDatabase final: public IDatabase {
    public:
        /**
         * @brief Structure holding PostgreSQL connection parameters
         *
         * Contains optional fields for various PostgreSQL connection options.
         * Any field that is not set will use PostgreSQL default values.
         */
        struct ConnectionInfo {
            std::optional< std::string > dbname;   /**< Database name */
            std::optional< std::string > user;     /**< Username for authentication */
            std::optional< std::string > password; /**< Password for authentication */
            std::optional< std::string > host;     /**< Host name or IP address */
            std::optional< std::string > port;     /**< Port number */

            /**
             * @brief Converts connection information to a connection string
             * @return PostgreSQL connection string with the specified parameters
             */
            std::string toString() const;
        };

    public:
        PsqlDatabase();
        ~PsqlDatabase() override;

        /**
         * @brief Connects to a PostgreSQL database using connection information structure
         *
         * @param connectionInfo The connection parameters
         * @return True if connection was successful, false otherwise
         */
        bool connect(const ConnectionInfo & connectionInfo);

        /**
         * @brief Connects to a PostgreSQL database using a connection string
         *
         * @param connectionString PostgreSQL connection string
         * @return True if connection was successful, false otherwise
         */
        bool connect(const std::string & connectionString);

        /**
         * @brief Closes the current database connection
         *
         * Releases any resources associated with the current connection.
         */
        void disconnect();

        // IDatabase interface implementation

        /**
         * @brief Creates a new transaction object for this database
         *
         * @return Shared pointer to an ITransaction interface for transaction operations
         */
        std::shared_ptr< ITransaction > makeTransaction() override;

        /**
         * @brief Checks if the database connection is valid and ready
         *
         * @return True if the database is connected and ready for operations, false otherwise
         */
        bool isReady() const noexcept override;

    private:
        /**
         * @brief PostgreSQL database connection handle
         *
         * Unique pointer to the underlying pqxx connection object.
         * Null if no connection is established.
         */
        std::unique_ptr< pqxx::connection > conn_;
    };

} // namespace cxx
