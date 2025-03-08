LIBRARY(jni-camera SHARED)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/camera/jni/jni_camera.h
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/camera/jni/jni_camera.cpp
)

LIBS(
  android

  cxx-camera
)

END()
