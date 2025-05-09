#include "doublespaces.h"

void cxx::removeDoubleSpaces(std::string & str) {
    std::size_t doubleSpace = str.find("  ");
    while (doubleSpace != std::string::npos) {
        str.erase(doubleSpace, 1);
        doubleSpace = str.find("  ");
    }
}
