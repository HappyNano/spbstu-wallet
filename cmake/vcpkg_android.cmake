# 1. Check the presence of environment variable ANDROID_NDK_HOME
if (NOT DEFINED ENV{ANDROID_NDK_HOME})
    message(FATAL_ERROR "
    Please set an environment variable ANDROID_NDK_HOME
    For example:
    export ANDROID_NDK_HOME=/home/your-account/Android/Sdk/ndk-bundle
    Or:
    export ANDROID_NDK_HOME=/home/your-account/Android/android-ndk-r21b
    ")
endif()

# Verify ANDROID_NDK path
if (NOT EXISTS "$ENV{ANDROID_NDK_HOME}/sources/android")
  message(FATAL_ERROR "Please set a valid ANDROID_NDK_HOME path")
endif()

# 2. Check the presence of environment variable VCPKG_ROOT
if (NOT DEFINED ENV{VCPKG_ROOT})
    message(FATAL_ERROR "
    Please set an environment variable VCPKG_ROOT
    For example:
    export VCPKG_ROOT=/path/to/vcpkg
    ")
endif()


# 3. Set VCPKG_TARGET_TRIPLET according to ANDROID_ABI
#
# There are four different Android ABI, each of which maps to
# a vcpkg triplet. The following table outlines the mapping from vcpkg architectures to android architectures
#
# |VCPKG_TARGET_TRIPLET       | ANDROID_ABI          |
# |---------------------------|----------------------|
# |arm64-android              | arm64-v8a            |
# |arm-android                | armeabi-v7a          |
# |x64-android                | x86_64               |
# |x86-android                | x86                  |
#
# The variable must be stored in the cache in order to successfuly the two toolchains.
if (ANDROID_ABI MATCHES "arm64-v8a")
    set(VCPKG_TARGET_TRIPLET "arm64-android" CACHE STRING "" FORCE)
elseif(ANDROID_ABI MATCHES "armeabi-v7a")
    set(VCPKG_TARGET_TRIPLET "arm-android" CACHE STRING "" FORCE)
elseif(ANDROID_ABI MATCHES "x86_64")
    set(VCPKG_TARGET_TRIPLET "x64-android" CACHE STRING "" FORCE)
elseif(ANDROID_ABI MATCHES "x86")
    set(VCPKG_TARGET_TRIPLET "x86-android" CACHE STRING "" FORCE)
else()
    message(FATAL_ERROR "
    Please specify ANDROID_ABI
    For example
    cmake ... -DANDROID_ABI=armeabi-v7a

    Possible ABIs are: arm64-v8a, armeabi-v7a, x64-android, x86-android
    ")
endif()
message("vcpkg_android.cmake: VCPKG_TARGET_TRIPLET was set to ${VCPKG_TARGET_TRIPLET}")

# 4. Combine vcpkg and Android toolchains

# vcpkg and android both provide dedicated toolchains:
#
# vcpkg_toolchain_file=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
# android_toolchain_file=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
#
# When using vcpkg, the vcpkg toolchain shall be specified first.
# However, vcpkg provides a way to preload and additional toolchain,
# with the VCPKG_CHAINLOAD_TOOLCHAIN_FILE option.
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE $ENV{ANDROID_NDK_HOME}/build/cmake/android.toolchain.cmake)
message("vcpkg_android.cmake: VCPKG_CHAINLOAD_TOOLCHAIN_FILE was set to ${VCPKG_CHAINLOAD_TOOLCHAIN_FILE}")
