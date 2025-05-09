LIBRARY(utils-config)

SRCS(
  ${PROJECT_SOURCE_DIR}/utils/config/config.h
  ${PROJECT_SOURCE_DIR}/utils/config/config.cpp
)

LIBS(
  utils-config-interfaces
)

END()

# ADD_TESTS(tests)

add_subdirectory(interfaces)
