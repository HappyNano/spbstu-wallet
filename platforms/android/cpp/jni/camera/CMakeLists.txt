LIBRARY(jni-camera SHARED)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/jni/camera/camera.h
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/jni/camera/camera.cpp
)

LIBS(
  imgui::imgui
  ${OpenCV_LIBS}
  GLESv3
  android
  utils-singleton
)

END()

target_include_directories(jni-camera PRIVATE
  ${OpenCV_INCLUDE_DIRS}
  ${Stb_INCLUDE_DIR}
)
