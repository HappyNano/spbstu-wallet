macro(__RESET)
  set(SUIT )
  set(SOURCES )
  set(LIBRARIES )
  set(INCLUDEDIRECTORIES )
  set(USECLANGTIDY ON)
endmacro()

macro(EXECUTABLE)
  if(ARGC GREATER 1)
    message(FATAL_ERROR "Macro EXECUTABLE: Bad args")
  endif()
  __RESET()
  set(SUIT "EXECUTABLE")
  set(EXECUTABLE_NAME ${ARGV0})
endmacro()

macro(LIBRARY)
  if(ARGC GREATER 2)
    message(FATAL_ERROR "Macro LIBRARY: Bad args")
  endif()
  __RESET()
  set(SUIT "LIBRARY")
  set(LIBRARY_NAME ${ARGV0})
  set(LIBRARY_PARAM ${ARGV1})
endmacro()

macro(GTEST)
  if(ARGC GREATER 1)
    message(FATAL_ERROR "Macro GTEST: Bad args")
  endif()
  __RESET()
  set(SUIT "GTEST")
  set(GTEST_NAME ${ARGV0})
endmacro()

macro(PROTO)
  if(ARGC GREATER 2)
    message(FATAL_ERROR "Macro PROTO: Bad args")
    endif()
  __RESET()
  set(SUIT "PROTO")
  set(PROTO_PARAM ${ARGV0})
  set(USECLANGTIDY OFF)
endmacro()

macro(SRCS)
  list(APPEND SOURCES ${ARGN})
endmacro()

macro(LIBS)
  list(APPEND LIBRARIES ${ARGN})
endmacro()

macro(INCLUDEDIRS)
  list(APPEND INCLUDEDIRECTORIES ${ARGN})
endmacro()

macro(PROTOS_deprecated)
  foreach(PROTO_NAME ${ARGN})
    set(PROTO_SRCS proto_srcs_${PROTO_NAME})
    set(PROTO_HDRS proto_hdrs_${PROTO_NAME})
    set(PROTO_PRAM proto_pram_${PROTO_NAME})
    if(NOT DEFINED ${PROTO_SRCS} OR NOT DEFINED ${PROTO_HDRS})
      message(FATAL_ERROR "Sources or headers for ${PROTO_NAME} not defined")
    endif()
    list(APPEND SOURCES ${${PROTO_SRCS}} ${${PROTO_HDRS}})
  endforeach()
  list(APPEND INCLUDEDIRECTORIES ${PROTOBUF_INCLUDE_DIRS} ${PROJECT_BINARY_DIR})
  list(APPEND LIBRARIES protobuf::libprotobuf-lite)
  if(${PROTO_PRAM} STREQUAL "GRPC")
    list(APPEND LIBRARIES gRPC::grpc++)
  endif()
endmacro()

function(generate_protobuf_sources PROTO_FILE)
  # Filename without extension
  get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
  get_filename_component(PROTO_DIR ${PROTO_FILE} DIRECTORY)
  cmake_path(RELATIVE_PATH PROTO_DIR BASE_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE PROTO_DIR_REL)
  string(REPLACE "\/" "_" PROTO_DIR_REL_NAME ${PROTO_DIR_REL})

  set(__PROTO_LIBRARIES )
  if(PROTO_PARAM STREQUAL "GRPC")
    list(APPEND __PROTO_LIBRARIES gRPC::grpc++_unsecure)

    set(PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.cc")
    set(PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.h")
    set(PROTO_SRCS ${PROTO_SRCS} "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc")
    set(PROTO_HDRS ${PROTO_HDRS} "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h")
    set(SOURCES ${PROTO_SRCS} ${PROTO_HDRS})

    find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
    add_custom_command(
      OUTPUT ${SOURCES}
      COMMAND ${Protobuf_PROTOC_EXECUTABLE}
      ARGS  --proto_path=${PROJECT_SOURCE_DIR}
            --grpc_out=${PROJECT_BINARY_DIR}
            --cpp_out=${PROJECT_BINARY_DIR}
            --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
            ${PROTO_FILE}
      DEPENDS ${PROTO_FILE}
      COMMENT "Generating gRPC and Protobuf files for ${PROTO_NAME}"
      VERBATIM
    )
  else()
    set(PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc")
    set(PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h")
    set(SOURCES ${PROTO_SRCS} ${PROTO_HDRS})

    add_custom_command(
      OUTPUT ${SOURCES}
      COMMAND ${Protobuf_PROTOC_EXECUTABLE}
      ARGS  --proto_path=${PROJECT_SOURCE_DIR}
            --cpp_out=${PROJECT_BINARY_DIR}
            ${PROTO_FILE}
      DEPENDS ${PROTO_FILE}
      COMMENT "Generating Protobuf files for ${PROTO_NAME}"
      VERBATIM
    )
  endif()
  list(APPEND __PROTO_LIBRARIES protobuf::libprotobuf) # !!! PROTO after grpc

  set(LIBRARY_NAME lib_${PROTO_DIR_REL_NAME}_${PROTO_NAME})
  add_library(${LIBRARY_NAME} ${SOURCES})
  target_link_libraries(${LIBRARY_NAME} PUBLIC ${__PROTO_LIBRARIES} ${LIBRARIES})
  target_include_directories(${LIBRARY_NAME} PUBLIC ${PROJECT_BINARY_DIR})
  if(NOT USECLANGTIDY)
    set_target_properties(${LIBRARY_NAME} PROPERTIES CXX_CLANG_TIDY "")
  endif()

  set(proto_srcs_${PROTO_DIR_REL_NAME}_${PROTO_NAME} ${PROTO_SRCS} CACHE PATH SRCS)
  set(proto_hdrs_${PROTO_DIR_REL_NAME}_${PROTO_NAME} ${PROTO_HDRS} CACHE PATH HDRS)
  set(proto_pram_${PROTO_DIR_REL_NAME}_${PROTO_NAME} ${PROTO_PARAM} CACHE PATH PRAM)

  set(ALL_PROTO_SRCS ${ALL_PROTO_SRCS} ${PROTO_SRCS} CACHE PATH ALLSRCS)
  set(ALL_PROTO_HDRS ${ALL_PROTO_HDRS} ${PROTO_HDRS} CACHE PATH ALLHDRS)
endfunction()

macro(END)
  if(SUIT STREQUAL "GTEST")
    add_executable(${GTEST_NAME}_test ${SOURCES})
    target_link_libraries(${GTEST_NAME}_test PUBLIC ${LIBRARIES} GTest::GTest GTest::Main)

    add_test(
      NAME ${GTEST_NAME}
      COMMAND ${GTEST_NAME}_test
    )
  elseif(SUIT STREQUAL "LIBRARY")
    add_library(${LIBRARY_NAME} ${LIBRARY_PARAM})
    target_sources(${LIBRARY_NAME} PRIVATE ${SOURCES})
    if(NOT ${LIBRARY_PARAM} STREQUAL "INTERFACE")
      set(LIBRARY_PARAM "PUBLIC")
    endif()
    target_link_libraries(${LIBRARY_NAME} ${LIBRARY_PARAM} ${LIBRARIES})
    if(INCLUDEDIRECTORIES)
      target_include_directories(${LIBRARY_NAME} PRIVATE ${INCLUDEDIRECTORIES})
      unset(INCLUDEDIRECTORIES)
    endif()
    if(NOT USECLANGTIDY)
      set_target_properties(${LIBRARY_NAME} PROPERTIES CXX_CLANG_TIDY "")
    endif()
  elseif(SUIT STREQUAL "EXECUTABLE")
    add_executable(${EXECUTABLE_NAME})
    target_sources(${EXECUTABLE_NAME} PRIVATE ${SOURCES})
    target_link_libraries(${EXECUTABLE_NAME} PUBLIC ${LIBRARIES})
    if(INCLUDEDIRECTORIES)
      target_include_directories(${EXECUTABLE_NAME} PRIVATE ${INCLUDEDIRECTORIES})
      unset(INCLUDEDIRECTORIES)
    endif()
    if(NOT USECLANGTIDY)
      set_target_properties(${EXECUTABLE_NAME} PROPERTIES CXX_CLANG_TIDY "")
    endif()
  elseif(SUIT STREQUAL "PROTO")
    foreach(PROTO_FILE ${SOURCES})
      generate_protobuf_sources(${PROTO_FILE})
    endforeach()
  else()
    message(FATAL_ERROR "Error")
  endif()

  set(SUIT "")
endmacro()

macro(ADD_TESTS)
  if(ARGC GREATER 1)
    message(FATAL_ERROR "Macro ADD_TESTS: Bad args")
  endif()
  if(BUILD_TESTING)
    add_subdirectory(${ARGV0})
  endif()
endmacro()
