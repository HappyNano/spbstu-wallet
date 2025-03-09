LIBRARY(cxx-logger INTERFACE)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/logger/logger.h
)

LIBS(
  android
)

END()
