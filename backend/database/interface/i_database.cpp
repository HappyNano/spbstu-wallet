#include "i_database.h"

std::string_view cxx::dataTypeToSql(Col::EDataType type) {
    switch (type) {
    case Col::EDataType::INTEGER:
        return "INTEGER";
    case Col::EDataType::REAL:
        return "REAL";
    case Col::EDataType::TEXT:
        return "TEXT";
    case Col::EDataType::BOOLEAN:
        return "BOOLEAN";
    case Col::EDataType::DATE:
        return "DATE";
    case Col::EDataType::TIMESTAMP:
        return "TIMESTAMP";
    default:
        return "TEXT";
    }
}

std::string_view cxx::constraintToSql(Col::EConstraint constraint) {
    switch (constraint) {
    case Col::EConstraint::PRIMARY_KEY:
        return "PRIMARY KEY";
    case Col::EConstraint::UNIQUE:
        return "UNIQUE";
    case Col::EConstraint::NOT_NULL:
        return "NOT NULL";
    case Col::EConstraint::FOREIGN_KEY:
        return "REFERENCES";
    default:
        return "";
    }
}
