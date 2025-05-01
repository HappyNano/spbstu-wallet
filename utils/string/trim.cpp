#include "trim.h"

#include <algorithm>
#include <cctype>

void cxx::ltrim(std::string & s) {
    s.erase(
     s.begin(),
     std::find_if(
      s.begin(),
      s.end(),
      [](unsigned char ch) {
          return !std::isspace(ch);
      }));
}

// trim from end (in place)
void cxx::rtrim(std::string & s) {
    s.erase(
     std::find_if(
      s.rbegin(),
      s.rend(),
      [](unsigned char ch) {
          return !std::isspace(ch);
      })
      .base(),
     s.end());
}

std::string cxx::ltrimCopy(std::string s) {
    ltrim(s);
    return s;
}

std::string cxx::rtrimCopy(std::string s) {
    rtrim(s);
    return s;
}
