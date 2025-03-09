#pragma once

#include <map>
#include <string_view>

namespace util {
    /**
     * @brief SQL Database interface class
     */
    class IDatabase {
    public:
        enum class EDataType {
            TEXT,

            BOOL,
            BINARY,
            TINYINT,   // Very small integer. Signed range from -128 to 127
            SMALLINT,  // Small integer. Signed range from -32768 to 32767
            MEDIUMINT, // Medium integer. Signed range from -8388608 to 8388607
            INT,       // Integer. Signed range is from -2147483648 to 2147483647
            BIGINT,    // Large integer. Signed range is from -9223372036854775808 to 9223372036854775807

            FLOAT,  // Floating point number
            DOUBLE, // Normal-size floating point number.

            DATE,      // Date. Format: YYYY-MM-DD.
            DATETIME,  // Date and time combination. Format: YYYY-MM-DD hh:mm:ss.
            TIMESTAMP, // Timestamp.
            TIME,      // Time. Format: hh:mm:ss.
        };

        enum class EConstraint {
            NOT_NULL,    // Ensures that a column cannot have a NULL value
            UNIQUE,      // Ensures that all values in a column are different
            PRIMARY_KEY, // A combination of a NOT NULL and UNIQUE. Uniquely identifies each row in a table
            FOREIGN_KEY, // Prevents actions that would destroy links between tables
            CHECK,       // Ensures that the values in a column satisfies a specific condition
            DEFAULT,     // Sets a default value for a column if no value is specified
        };

        struct ColParams {
            EDataType dataType;
            EConstraint constraint = EConstraint::DEFAULT;
        };

    public:
        virtual ~IDatabase();

        virtual void createTable(std::string_view tableName, std::map< std::string, ColParams > cols) = 0;
        virtual void dropTable(std::string_view tableName) = 0;

        virtual void insertRow() = 0;
        virtual void upsertRow() = 0;
    };
} // namespace util
