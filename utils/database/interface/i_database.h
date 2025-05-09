#pragma once

#include <utils/database/transaction/interface/i_transaction.h>

#include <memory>

namespace cxx {

    /**
     * @class IDatabase
     * @brief Interface defining the common operations for database access.
     *
     * This interface provides an abstraction layer for database operations,
     *
     * allowing different database implementations to be used interchangeably.
     */
    class IDatabase {
    public:
        /**
         * @brief Virtual destructor for proper cleanup of derived classes.
         */
        virtual ~IDatabase();

        /**
         * @brief Creates a new database transaction.
         * Transactions provide a way to group database operations together
         * to ensure atomicity, consistency, isolation, and durability.
         * @return std::shared_ptr<ITransaction> A shared pointer to a new transaction object.
         */
        virtual std::shared_ptr< ITransaction > makeTransaction() = 0;

        /**
         *@brief Checks if the database is ready for operations.
         *Verifies that the database is properly initialized and can
         *accept queries and other operations.
         *@return bool True if the database is ready, false otherwise.
         */
        virtual bool isReady() const noexcept = 0;
    };

} // namespace cxx
