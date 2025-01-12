cmake_minimum_required(VERSION 3.12)

project(WALLET VERSION 0 LANGUAGES CXX)

if ("${PROJECT_BINARY_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}")
  message(FATAL_ERROR "In-source builds are not allowed.
    Please create a subfolder and use `cmake ..` inside it.
    NOTE: cmake creates CMakeCache.txt and CMakeFiles/*.
          Remove them, or cmake will refuse to work.")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS Off)
set(CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_CXX_CLANG_TIDY clang-tidy --config-file=${PROJECT_SOURCE_DIR}/.clang-tidy)

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

find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(utils)
add_subdirectory(wallet)
