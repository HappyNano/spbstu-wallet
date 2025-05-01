#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace cxx {

    struct Col {
        std::string name;
        enum class EDataType {
            INTEGER,
            REAL,
            TEXT,
            BOOLEAN,
            DATE,
            TIMESTAMP
        } type;

        enum class EConstraint {
            NONE,
            PRIMARY_KEY,
            UNIQUE,
            NOT_NULL,
            FOREIGN_KEY
        } constraint = EConstraint::NONE;
    };

    std::string_view dataTypeToSql(Col::EDataType type);
    std::string_view constraintToSql(Col::EConstraint constraint);

    using QueryResult = std::vector< std::vector< std::variant< int, double, std::string, bool > > >;

    class IDatabase {
    public:
        virtual ~IDatabase() = default;

        // Подключение к базе данных
        virtual bool connect(const std::string & connection_string) = 0;

        // Отключение от базы данных
        virtual void disconnect() = 0;

        // Создание таблицы
        virtual bool createTable(const std::string & name, const std::vector< Col > & cols) = 0;

        // Удаление таблицы
        virtual bool dropTable(const std::string & tableName) = 0;

        // Выборка данных
        virtual std::optional< QueryResult > select(const std::string & fromTableName, const std::vector< std::string > & colsName) = 0;

        // Вставка данных
        virtual bool insert(const std::string & tableName, const std::vector< std::string > & colNames, const std::vector< std::string > & values) = 0;

        // Обновление данных
        virtual bool update(const std::string & tableName, const std::vector< std::pair< std::string, std::string > > & colValuePairs, const std::string & whereCondition) = 0;

        // Удаление данных
        virtual bool deleteFrom(const std::string & tableName, const std::string & whereCondition) = 0;

        // Проверка существования таблицы
        virtual bool isTableExist(const std::string & tableName) = 0;

        // Выполнение произвольного SQL-запроса
        virtual std::optional< QueryResult > executeQuery(const std::string & query) = 0;
    };

} // namespace cxx
