LIBRARY(android-lib SHARED)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/main.cpp
)

LIBS(
  imgui::imgui

  android
  EGL
  GLESv3

  app-glue
  android-cxx-camera
  android-cxx-camera-jni
  android-cxx-main_activity
  common-main_activity-lib
  common-main_loop
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
add_subdirectory(main_activity)
