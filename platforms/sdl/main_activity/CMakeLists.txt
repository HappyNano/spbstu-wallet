LIBRARY(sdl-main_activity)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/sdl/main_activity/sdl_main_activity.cpp
  ${PROJECT_SOURCE_DIR}/platforms/sdl/main_activity/sdl_main_activity.h
)

LIBS(
  common-main_activity

  imgui::imgui
  ${SDL}
  spdlog::spdlog
)

END()
