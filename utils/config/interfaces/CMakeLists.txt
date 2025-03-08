LIBRARY(utils-config-interfaces INTERFACE)

SRCS(
  ${PROJECT_SOURCE_DIR}/utils/config/interfaces/i_config.h
)

LIBS(
  nlohmann_json::nlohmann_json
)

END()
