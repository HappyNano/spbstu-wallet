cmake_minimum_required(VERSION 3.12)

if(PLATFORM STREQUAL "ANDROID")
  if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(FATAL_ERROR "Only Debug builds are supported")
  endif()
  include(${CMAKE_SOURCE_DIR}/cmake/vcpkg_android.cmake)
  list(APPEND VCPKG_MANIFEST_FEATURES "android")  # Feature in vcpkg.json
elseif(PLATFORM STREQUAL "LINUX" OR PLATFORM STREQUAL "WIN" OR PLATFORM STREQUAL "APPLE")
  list(APPEND VCPKG_MANIFEST_FEATURES "other-platform")  # Feature in vcpkg.json
elseif(PLATFORM STREQUAL "BACKEND")
  list(APPEND VCPKG_MANIFEST_FEATURES "backend")  # Feature in vcpkg.json
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
find_program(CTIDY_FOUND clang-tidy-19)
set(CLANG_TIDY_EXECUTABLE clang-tidy)
if(CTIDY_FOUND)
  set(CLANG_TIDY_EXECUTABLE clang-tidy-19)
endif()
set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_EXECUTABLE} --config-file=${PROJECT_SOURCE_DIR}/.clang-tidy)

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
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif()

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(utils)
add_subdirectory(proto)
if(PLATFORM STREQUAL "BACKEND")
  add_subdirectory(backend)
else()
  find_package(imgui REQUIRED)
  find_package(Stb REQUIRED)
  find_package(OpenCV 4 REQUIRED)
  add_subdirectory(platforms)
endif()
