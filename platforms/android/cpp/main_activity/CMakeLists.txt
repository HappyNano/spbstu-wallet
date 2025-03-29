LIBRARY(android-cxx-main_activity)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/main_activity/android_main_activity.h
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/main_activity/android_main_activity.cpp
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/main_activity/jni_executor.h
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/main_activity/jni_executor.cpp
)

LIBS(
  ${OpenCV_LIBS}
  imgui::imgui
  spdlog::spdlog
  android

  utils-singleton
)

INCLUDEDIRS(
  ${OpenCV_INCLUDE_DIRS}
  $ENV{ANDROID_NDK_HOME}/sources/android/native_app_glue
)

END()

target_compile_definitions(android-cxx-main_activity PRIVATE
  IMGUI_IMPL_OPENGL_ES3
)
