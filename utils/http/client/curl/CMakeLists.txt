LIBRARY(http_client_curl)

SRCS(
  ${PROJECT_SOURCE_DIR}/utils/http/client/curl/curl_http_client.h
  ${PROJECT_SOURCE_DIR}/utils/http/client/curl/curl_http_client.cpp
)

LIBS(
  http_client_interface
  utils_string

  CURL::libcurl
)

END()

ADD_TESTS(tests)
