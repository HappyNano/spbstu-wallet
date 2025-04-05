cmake_minimum_required(VERSION 3.12)

if(PLATFORM STREQUAL "ANDROID")
  include(${CMAKE_SOURCE_DIR}/cmake/vcpkg_android.cmake)
  list(APPEND VCPKG_MANIFEST_FEATURES "android")  # Feature in vcpkg.json
elseif(PLATFORM STREQUAL "LINUX" OR PLATFORM STREQUAL "WIN" OR PLATFORM STREQUAL "APPLE")
  list(APPEND VCPKG_MANIFEST_FEATURES "other-platform")  # Feature in vcpkg.json
endif()
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)

project(WALLET VERSION 0 LANGUAGES C CXX)

if ("${PROJECT_BINARY_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")
  message(FATAL_ERROR "In-source builds are not allowed.
    Please create a subfolder and use `cmake ..` inside it.
    NOTE: cmake creates CMakeCache.txt and CMakeFiles/*.
          Remove them, or cmake will refuse to work.")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_CXX_CLANG_TIDY clang-tidy-19 --config-file=${PROJECT_SOURCE_DIR}/.clang-tidy)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/cmake)

include(compiler_flags)
include(gitmodules)
include(utils)

option(BUILD_TESTING "" OFF)
if(BUILD_TESTING)
  find_package(GTest REQUIRED)
  list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
  enable_testing()
endif()

find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(imgui REQUIRED)
find_package(Stb REQUIRED)
find_package(OpenCV 4 REQUIRED)
find_package(Protobuf REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(utils)
add_subdirectory(wallet)
add_subdirectory(proto)
add_subdirectory(backend)
add_subdirectory(platforms)
