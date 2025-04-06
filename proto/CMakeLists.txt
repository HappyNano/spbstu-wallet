PROTO()

SRCS(
  ${PROJECT_SOURCE_DIR}/proto/test.proto
)

END()

add_subdirectory(testdir)
add_subdirectory(helloworld)
add_subdirectory(database)
add_subdirectory(wallet)
