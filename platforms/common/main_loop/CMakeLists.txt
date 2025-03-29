LIBRARY(common-main_loop)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/common/main_loop/main_loop.cpp
  ${PROJECT_SOURCE_DIR}/platforms/common/main_loop/main_loop.h
)

LIBS(
  ${OpenCV_LIBS}
  imgui::imgui

  EGL
  GLESv3
)

INCLUDEDIRS(
  ${OpenCV_INCLUDE_DIRS}
)

END()
