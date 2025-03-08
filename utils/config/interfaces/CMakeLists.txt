LIBRARY(utils-config-interfaces)

SRCS(
  ${PROJECT_SOURCE_DIR}/utils/config/interfaces/i_config.h
  ${PROJECT_SOURCE_DIR}/utils/config/interfaces/i_config.cpp
)

LIBS(
  nlohmann_json::nlohmann_json
)

END()
