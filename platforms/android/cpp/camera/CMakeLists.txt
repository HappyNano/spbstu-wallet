LIBRARY(cxx-camera)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/camera/camera.h
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/camera/camera.cpp
)

LIBS(
  ${OpenCV_LIBS}
  imgui::imgui
  android

  utils-singleton
)

INCLUDEDIRS(
  ${OpenCV_INCLUDE_DIRS}
)

END()

add_subdirectory(jni)
