LIBRARY(android-lib SHARED)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/main.cpp
)

LIBS(
  app-glue
  cxx-camera
  jni-camera
  cxx-main_activity

  imgui::imgui

  android
  EGL
  GLESv3
  log
)

INCLUDEDIRS(
  $ENV{ANDROID_NDK_HOME}/sources/android/native_app_glue
)

END()

set(CMAKE_SHARED_LINKER_FLAGS
  "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate"
)

target_compile_definitions(android-lib PRIVATE
  IMGUI_IMPL_OPENGL_ES3
)

add_subdirectory(app_glue)
add_subdirectory(camera)
add_subdirectory(logger)
add_subdirectory(main_activity)
