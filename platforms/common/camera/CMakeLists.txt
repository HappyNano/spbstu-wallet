LIBRARY(common-camera-lib)

SRCS(
  ${PROJECT_SOURCE_DIR}/platforms/common/camera/i_camera.cpp
  ${PROJECT_SOURCE_DIR}/platforms/common/camera/i_camera.h
  ${PROJECT_SOURCE_DIR}/platforms/common/camera/null_camera.cpp
  ${PROJECT_SOURCE_DIR}/platforms/common/camera/null_camera.h
  ${PROJECT_SOURCE_DIR}/platforms/common/camera/camera_sink.cpp
  ${PROJECT_SOURCE_DIR}/platforms/common/camera/camera_sink.h
  ${PROJECT_SOURCE_DIR}/platforms/common/camera/frame.cpp
  ${PROJECT_SOURCE_DIR}/platforms/common/camera/frame.h
)

LIBS(
  ${OpenCV_LIBS}
  imgui::imgui
)

INCLUDEDIRS(
  ${OpenCV_INCLUDE_DIRS}
)

END()
