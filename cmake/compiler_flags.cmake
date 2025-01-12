set(_GNU_COMMON_C_CXX_FLAGS "\
  -fexceptions \
  -fno-common \
  -ffunction-sections \
  -fdata-sections \
  -Wall \
  -Wextra \
  -Wno-parentheses \
  -Wno-implicit-const-int-float-conversion \
  -Wno-unknown-warning-option \
  -pipe \
  -pthread \
  -D__STDC_CONSTANT_MACROS \
  -D__STDC_FORMAT_MACROS \
  -D__LONG_LONG_SUPPORTED \
")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${_GNU_COMMON_C_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${_GNU_COMMON_C_CXX_FLAGS} \
  -Woverloaded-virtual \
  -Wno-undefined-var-template \
  -Wno-return-std-move \
  -Wno-defaulted-function-deleted \
  -Wno-pessimizing-move \
  -Wno-deprecated-anon-enum-enum-conversion \
  -Wno-deprecated-enum-enum-conversion \
  -Wno-deprecated-enum-float-conversion \
  -Wno-ambiguous-reversed-operator \
  -Wno-deprecated-volatile \
")
