GTEST("http_client_curl")

SRCS(
  ${PROJECT_SOURCE_DIR}/utils/http/client/curl/tests/curl_http_client_test.cpp
)

LIBS(
  http_client_curl
  http_server_test
)

END()
