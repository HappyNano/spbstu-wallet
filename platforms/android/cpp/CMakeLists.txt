LIBRARY(android-lib SHARED)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/android/cpp/main.cpp
)

LIBS(
  app-glue
  android-cxx-camera
  android-cxx-jni-camera
  android-cxx-jni-camera_sink
  android-cxx-main_activity
  common-main_loop
  common-client

  android
  spdlog::spdlog
)

INCLUDEDIRS(
  $ENV{ANDROID_NDK_HOME}/sources/android/native_app_glue
)

END()

set(CMAKE_SHARED_LINKER_FLAGS
  "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate"
)

add_subdirectory(app_glue)
add_subdirectory(camera)
add_subdirectory(main_activity)
