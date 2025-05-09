#pragma once

#include <jni.h>

extern "C" JNIEXPORT void JNICALL
 // NOLINTNEXTLINE(readability-identifier-naming)
 Java_imgui_example_android_CameraHelper_onImageAvailable(JNIEnv * env, jclass, jbyteArray data, jint width, jint height);
