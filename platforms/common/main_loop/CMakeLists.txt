LIBRARY(common-main_loop)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/common/main_loop/main_loop.cpp
  ${PROJECT_SOURCE_DIR}/platforms/common/main_loop/main_loop.h
  ${PROJECT_SOURCE_DIR}/platforms/common/main_loop/context.cpp
  ${PROJECT_SOURCE_DIR}/platforms/common/main_loop/context.h
)

LIBS(
  common-camera-lib
  common-client_interface

  ${OpenCV_LIBS}
  imgui::imgui
  spdlog::spdlog
)

INCLUDEDIRS(
  ${OpenCV_INCLUDE_DIRS}
)

END()
