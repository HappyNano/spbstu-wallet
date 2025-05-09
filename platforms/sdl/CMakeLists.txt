if(PLATFORM STREQUAL "LINUX")
  # include(CMakeLists.linux.txt)
elseif(PLATFORM STREQUAL "WIN")
  # include(CMakeLists.windows.txt)
elseif(PLATFORM STREQUAL "APPLE")
  # include(CMakeLists.apple.txt)
else()
  message(FATAL_ERROR "To build sdl-platform choose PLATFORM (LINUX, WIN, APPLE)")
endif()

EXECUTABLE(main_exe)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/sdl/main.cpp
)

LIBS(
  sdl-main_activity
  common-main_loop

  imgui::imgui
  spdlog::spdlog
)

END()

add_subdirectory(main_activity)
