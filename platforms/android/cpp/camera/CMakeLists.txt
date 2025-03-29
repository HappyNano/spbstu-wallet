LIBRARY(android-cxx-camera)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/camera/camera.h
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/camera/camera.cpp
)

LIBS(
  android-cxx-main_activity
  common-camera-lib

  spdlog::spdlog
)

END()

add_subdirectory(jni)
