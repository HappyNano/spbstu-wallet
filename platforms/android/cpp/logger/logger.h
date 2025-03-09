#pragma once

#include <android/log.h>

#include <utility>

namespace cxx {
    class AndroidLogger final {
    public:
        static constexpr const char * LOG_TAG = "AndroidCppLib";

        template < typename... Args >
        static void log(int type, const char * fmt, Args... args) {
            __android_log_print(type, LOG_TAG, fmt, std::forward< Args >(args)...);
        }
        template < typename... Args >
        static void logError(const char * fmt, Args... args) {
            log(ANDROID_LOG_ERROR, fmt, std::forward< Args >(args)...);
        }
        template < typename... Args >
        static void logInfo(const char * fmt, Args... args) {
            log(ANDROID_LOG_INFO, fmt, std::forward< Args >(args)...);
        }
    };
} // namespace cxx
