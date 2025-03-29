LIBRARY(android-cxx-camera-jni SHARED)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/camera/jni/jni_camera.h
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/camera/jni/jni_camera.cpp
)

LIBS(
  android-cxx-camera_sink-jni

  spdlog::spdlog
  android
)

END()

add_subdirectory(sink)
