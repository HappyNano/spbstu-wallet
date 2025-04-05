macro(EXECUTABLE)
  if(ARGC GREATER 1)
    message(FATAL_ERROR "Macro EXECUTABLE: Bad args")
  endif()
  set(SUIT "EXECUTABLE")
  set(EXECUTABLE_NAME ${ARGV0})
  set(SOURCES )
  set(LIBRARIES )
  set(INCLUDEDIRECTORIES )
endmacro()

macro(LIBRARY)
  if(ARGC GREATER 2)
    message(FATAL_ERROR "Macro LIBRARY: Bad args")
  endif()
  set(SUIT "LIBRARY")
  set(LIBRARY_NAME ${ARGV0})
  set(LIBRARY_PARAM ${ARGV1})
  set(SOURCES )
  set(LIBRARIES )
  set(INCLUDEDIRECTORIES )
endmacro()

macro(GTEST)
  if(ARGC GREATER 1)
    message(FATAL_ERROR "Macro GTEST: Bad args")
  endif()
  set(SUIT "GTEST")
  set(GTEST_NAME ${ARGV0})
  set(SOURCES )
  set(LIBRARIES )
  set(INCLUDEDIRECTORIES )
endmacro()

macro(PROTO)
  if(ARGC GREATER 2)
    message(FATAL_ERROR "Macro PROTO: Bad args")
  endif()
  set(SUIT "PROTO")
  set(PROTO_PARAM ${ARGV0})
  set(SOURCES )
endmacro()

macro(SRCS)
  set(SOURCES ${ARGN})
endmacro()

macro(LIBS)
  set(LIBRARIES ${ARGN})
endmacro()

macro(INCLUDEDIRS)
  set(INCLUDEDIRECTORIES ${ARGN})
endmacro()

macro(PROTOS)
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

  if(PROTO_PARAM STREQUAL "GRPC")
    set(PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.cc")
    set(PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.h")
    set(PROTO_SRCS ${PROTO_SRCS} "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc")
    set(PROTO_HDRS ${PROTO_HDRS} "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h")
    find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
    execute_process(
      COMMAND ${Protobuf_PROTOC_EXECUTABLE}
      --proto_path=${PROTO_DIR}
      --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
      --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
      --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
      ${PROTO_FILE}
    )
  else()
    set(PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc")
    set(PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h")
    execute_process(
      COMMAND ${Protobuf_PROTOC_EXECUTABLE}
      --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
      --proto_path=${PROTO_DIR}
      ${PROTO_FILE}
    )
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
  elseif(SUIT STREQUAL "EXECUTABLE")
    add_executable(${EXECUTABLE_NAME})
    target_sources(${EXECUTABLE_NAME} PRIVATE ${SOURCES})
    target_link_libraries(${EXECUTABLE_NAME} PUBLIC ${LIBRARIES})
    if(INCLUDEDIRECTORIES)
      target_include_directories(${EXECUTABLE_NAME} PRIVATE ${INCLUDEDIRECTORIES})
      unset(INCLUDEDIRECTORIES)
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
