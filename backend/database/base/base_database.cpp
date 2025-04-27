#include "base_database.h"
#include "backend/database/interface/i_database.h"

#include <optional>
#include <sstream>

using namespace cxx;

auto BaseDatabase::callWithCheck(auto fn, auto returnFailVal) {
    return isReady() ? fn() : returnFailVal;
}

bool BaseDatabase::createTable(const std::string & name, const std::vector< Col > & cols) {
    return callWithCheck(
     [this, &name, &cols] {
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

         auto result = executeQuery(query.str());
         return result.has_value();
     },
     false);
}

bool BaseDatabase::dropTable(const std::string & tableName) {
    return callWithCheck(
     [this, &tableName] {
         std::string query = "DROP TABLE " + escapeString(tableName) + ";";
         auto result = executeQuery(query);
         return result.has_value();
     },
     false);
}

std::optional< QueryResult > BaseDatabase::select(const std::string & fromTableName, const std::vector< std::string > & colsName) {
    return callWithCheck(
     [this, &fromTableName, &colsName] {
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

         return executeQuery(query.str());
     },
     std::nullopt);
}

bool BaseDatabase::insert(const std::string & tableName, const std::vector< std::string > & colNames, const std::vector< std::string > & values) {
    return callWithCheck(
     [this, &tableName, &colNames, &values] {
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

         auto result = executeQuery(query.str());
         return result.has_value();
     },
     false);
}

bool BaseDatabase::update(const std::string & tableName, const std::vector< std::pair< std::string, std::string > > & colValuePairs, const std::string & whereCondition) {
    return callWithCheck(
     [this, &tableName, &colValuePairs, &whereCondition] {
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

         auto result = executeQuery(query.str());
         return result.has_value();
     },
     false);
}

bool BaseDatabase::deleteFrom(const std::string & tableName, const std::string & whereCondition) {
    return callWithCheck(
     [this, &tableName, &whereCondition] {
         std::stringstream query;
         query << "DELETE FROM " << escapeString(tableName);

         if (!whereCondition.empty()) {
             query << " WHERE " << whereCondition;
         }

         query << ";";

         auto result = executeQuery(query.str());
         return result.has_value();
     },
     false);
}
