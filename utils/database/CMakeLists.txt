LIBRARY(utils-database)

SRCS(
  ${PROJECT_SOURCE_DIR}/utils/database/database.h
  ${PROJECT_SOURCE_DIR}/utils/database/database.cpp
)

LIBS(
  utils-database-interfaces
)

END()

# ADD_TESTS(tests)

add_subdirectory(interfaces)
