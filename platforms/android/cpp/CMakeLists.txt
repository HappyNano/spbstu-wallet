LIBRARY(android-lib SHARED)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/main.cpp
)

LIBS(
  jni-camera

  imgui::imgui

  app-glue
  android
  EGL
  GLESv3
  log
)

END()

set(CMAKE_SHARED_LINKER_FLAGS
  "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate"
)

target_compile_definitions(android-lib PRIVATE
  IMGUI_IMPL_OPENGL_ES3
)

target_include_directories(android-lib PRIVATE
  $ENV{ANDROID_NDK_HOME}/sources/android/native_app_glue
)

add_subdirectory(app_glue)
add_subdirectory(jni)
