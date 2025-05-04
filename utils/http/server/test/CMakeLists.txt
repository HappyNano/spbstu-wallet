LIBRARY(http_server_test)

SRCS(
  ${PROJECT_SOURCE_DIR}/utils/http/server/test/test_http_server.h
  ${PROJECT_SOURCE_DIR}/utils/http/server/test/test_http_server.cpp
)

LIBS(
  CURL::libcurl
  spdlog::spdlog
)

END()
