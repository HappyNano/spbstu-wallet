#include "jni_executor.h"

#include <spdlog/spdlog.h>

#include <string>

cxx::JNIExecutor::JNIExecutor(JavaVM * javaVm, jobject javaClassObject, std::string_view name, std::string_view sign)
  : javaVm_{ javaVm }
  , javaClassObject_{ javaClassObject } {
    jint jniReturn = javaVm->GetEnv((void **)&javaEnv_, JNI_VERSION_1_6);
    ENSURE(jniReturn != JNI_ERR, "Can't get env");

    jniReturn = javaVm->AttachCurrentThread(&javaEnv_, nullptr);
    ENSURE(jniReturn == JNI_OK, "Can't attach current thread");

    jclass nativeActivityClazz = javaEnv_->GetObjectClass(javaClassObject);
    ENSURE(nativeActivityClazz, "Can't get object class");

    methodId_ = javaEnv_->GetMethodID(nativeActivityClazz, name.data(), sign.data());
    ENSURE(methodId_, std::string("Can't get method ") + name.data());
}

cxx::JNIExecutor::~JNIExecutor() {
    jint jniReturn = javaVm_->DetachCurrentThread();
    if (jniReturn != JNI_OK) { // for noexcept
        SPDLOG_ERROR("~Executor:: Can't detach current thread");
    }
}

JNIEnv * cxx::JNIExecutor::getJavaEnv() {
    return javaEnv_;
}
