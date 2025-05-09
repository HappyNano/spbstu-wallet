#pragma once

#include <jni.h>

#include <stdexcept>
#include <string>
#include <string_view>

#define ENSURE(statement, error)             \
    {                                        \
        if (!(statement)) {                  \
            throw std::runtime_error(error); \
        }                                    \
    }

namespace cxx {
    class JNIExecutor {
    public:
        JNIExecutor(JavaVM * javaVm, jobject javaClassObject, std::string_view name, std::string_view sign);
        ~JNIExecutor();

        JNIEnv * getJavaEnv();

        template < typename Return, typename... Args >
        auto call(Args... args) -> Return;

    private:
        JavaVM * javaVm_;
        JNIEnv * javaEnv_;
        jobject javaClassObject_;
        jmethodID methodId_;
    };
} // namespace cxx

template < typename Return, typename... Args >
auto cxx::JNIExecutor::call(Args... args) -> Return {
    if constexpr (std::is_same_v< Return, void >) {
        javaEnv_->CallVoidMethod(javaClassObject_, methodId_, std::forward< Args >(args)...);
    } else if constexpr (std::is_same_v< Return, int > || std::is_same_v< Return, jint >) {
        return javaEnv_->CallIntMethod(javaClassObject_, methodId_, std::forward< Args >(args)...);
    } else if constexpr (std::is_same_v< Return, long > || std::is_same_v< Return, jlong >) {
        return javaEnv_->CallLongMethod(javaClassObject_, methodId_, std::forward< Args >(args)...);
    } else if constexpr (std::is_same_v< Return, bool > || std::is_same_v< Return, jboolean >) {
        return javaEnv_->CallBooleanMethod(javaClassObject_, methodId_, std::forward< Args >(args)...);
    } else if constexpr (std::is_same_v< Return, float > || std::is_same_v< Return, jfloat >) {
        return javaEnv_->CallFloatMethod(javaClassObject_, methodId_, std::forward< Args >(args)...);
    } else if constexpr (std::is_same_v< Return, double > || std::is_same_v< Return, jdouble >) {
        return javaEnv_->CallDoubleMethod(javaClassObject_, methodId_, std::forward< Args >(args)...);
    } else {
        throw std::runtime_error("There is no method with Return type " + std::string(typeid(Return).name()));
    }
}
