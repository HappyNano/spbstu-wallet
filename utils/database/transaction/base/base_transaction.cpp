#include "base_transaction.h"

#include <optional>
#include <sstream>

using namespace cxx;

bool BaseTransaction::createTable(const std::string & name, const std::vector< Col > & cols) {
    std::stringstream query;
    query << "CREATE TABLE " << escapeString(name) << " (";

    for (size_t i = 0; i < cols.size(); ++i) {
        const auto & col = cols[i];
        query << escapeString(col.name) << " " << dataTypeToSql(col.type);

        if (col.constraint != Col::EConstraint::NONE) {
            query << " " << constraintToSql(col.constraint);
        }

        if (i < cols.size() - 1) {
            query << ", ";
        }
    }

    query << ");";

    auto result = executeQueryUnsafe(query.str());
    return result.has_value();
}

bool BaseTransaction::dropTable(const std::string & tableName) {
    std::string query = "DROP TABLE " + escapeString(tableName) + ";";
    auto result = executeQueryUnsafe(query);
    return result.has_value();
}

std::optional< QueryResult > BaseTransaction::select(const std::string & fromTableName, const std::vector< std::string > & colsName) {
    std::stringstream query;
    query << "SELECT ";

    if (colsName.empty()) {
        query << "*";
    } else {
        for (size_t i = 0; i < colsName.size(); ++i) {
            query << escapeString(colsName[i]);
            if (i < colsName.size() - 1) {
                query << ", ";
            }
        }
    }

    query << " FROM " << escapeString(fromTableName) << ";";

    return executeQueryUnsafe(query.str());
}

bool BaseTransaction::insert(const std::string & tableName, const std::vector< std::string > & colNames, const std::vector< std::string > & values) {
    if (colNames.size() != values.size()) {
        return false;
    }

    std::stringstream query;
    query << "INSERT INTO " << escapeString(tableName) << " (";

    for (size_t i = 0; i < colNames.size(); ++i) {
        query << escapeString(colNames[i]);
        if (i < colNames.size() - 1) {
            query << ", ";
        }
    }

    query << ") VALUES (";

    for (size_t i = 0; i < values.size(); ++i) {
        query << "'" << escapeString(values[i]) << "'";
        if (i < values.size() - 1) {
            query << ", ";
        }
    }

    query << ");";

    auto result = executeQueryUnsafe(query.str());
    return result.has_value();
}

bool BaseTransaction::update(const std::string & tableName, const std::vector< std::pair< std::string, std::string > > & colValuePairs, const std::string & whereCondition) {
    if (colValuePairs.empty()) {
        return false;
    }

    std::stringstream query;
    query << "UPDATE " << escapeString(tableName) << " SET ";

    for (size_t i = 0; i < colValuePairs.size(); ++i) {
        query << escapeString(colValuePairs[i].first) << " = '" << escapeString(colValuePairs[i].second) << "'";
        if (i < colValuePairs.size() - 1) {
            query << ", ";
        }
    }

    if (!whereCondition.empty()) {
        query << " WHERE " << whereCondition;
    }

    query << ";";

    auto result = executeQueryUnsafe(query.str());
    return result.has_value();
}

bool BaseTransaction::deleteFrom(const std::string & tableName, const std::string & whereCondition) {
    std::stringstream query;
    query << "DELETE FROM " << escapeString(tableName);

    if (!whereCondition.empty()) {
        query << " WHERE " << whereCondition;
    }

    query << ";";

    auto result = executeQueryUnsafe(query.str());
    return result.has_value();
}

// Works for most SQL databases
bool BaseTransaction::isTableExist(const std::string & tableName) {
    std::stringstream query;
    query << "SELECT EXISTS (SELECT 1 FROM information_schema.tables WHERE table_name = '";
    query << escapeString(tableName);
    query << "');";

    auto result = executeQueryUnsafe(query.str());
    if (!result.has_value() || result->empty() || (*result)[0].empty()) {
        return false;
    }

    // Convert the result to boolean
    auto & value = (*result)[0][0];
    if (std::holds_alternative< std::string >(value)) {
        std::string stringVal = std::get< std::string >(value);
        return stringVal == "t" || stringVal == "true" || stringVal == "1";
    } else if (std::holds_alternative< bool >(value)) {
        return std::get< bool >(value);
    } else if (std::holds_alternative< int >(value)) {
        return std::get< int >(value) != 0;
    }

    return false;
}

std::optional< QueryResult > BaseTransaction::executeQuery(const std::string & query) {
    // TODO: some checks
    return executeQueryUnsafe(query);
}
