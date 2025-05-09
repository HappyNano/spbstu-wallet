LIBRARY(android-cxx-jni-camera SHARED)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/camera/jni/jni_camera.h
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/camera/jni/jni_camera.cpp
)

LIBS(
  android-cxx-jni-camera_sink

  spdlog::spdlog
  android
)

END()

add_subdirectory(sink)
