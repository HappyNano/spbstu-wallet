LIBRARY(platforms-common-main_activity-lib)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/common/main_activity/i_main_activity.cpp
  ${PROJECT_SOURCE_DIR}/platforms/common/main_activity/i_main_activity.h
)

LIBS(
  imgui::imgui

  EGL
  GLESv3
)

END()

# target_compile_definitions(android-lib PRIVATE
#   IMGUI_IMPL_OPENGL_ES3
# )
