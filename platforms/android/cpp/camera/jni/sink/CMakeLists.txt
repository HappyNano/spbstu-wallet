LIBRARY(android-cxx-camera_sink-jni)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/camera/jni/sink/jni_camera_sink.h
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/camera/jni/sink/jni_camera_sink.cpp
)

LIBS(
  common-camera-lib

  spdlog::spdlog
  android
)

END()
