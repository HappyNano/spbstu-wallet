#pragma once

#include <sqlite3.h>
#include <utils/database/interface/i_database.h>

 namespace cxx {

    /**
     * @brief SQLite implementation of the IDatabase interface
     *
     * This class provides SQLite-specific database functionality, including
     * connection management and transaction creation.
     */
    class SQLiteDatabase final: public IDatabase {
    public:
        SQLiteDatabase() = default;
        ~SQLiteDatabase() override;

        /**
         * @brief Creates an in-memory SQLite database
         *
         * Establishes a connection to an in-memory SQLite database, which exists
         * only for the duration of the connection.
         *
         * @return True if connection was successful, false otherwise
         */
        bool connectInMemory();

        /**
         * @brief Connects to a SQLite database file
         *
         * @param connectionInfo Path to the SQLite database file
         * @return True if connection was successful, false otherwise
         */
        bool connect(const std::string & connectionInfo);

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
         * @brief SQLite database connection handle
         *
         * Pointer to the underlying SQLite database connection.
         * Null if no connection is established.
         */
        sqlite3 * conn_ = nullptr;
    };

} // namespace cxx
