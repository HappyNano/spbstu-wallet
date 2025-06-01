#include "jni_camera_sink.h"

#include <spdlog/spdlog.h>

#include <memory>

auto cxx::JniCameraSink::loadNewFrameFromJni(jbyte * data, int width, int height, int channels) -> void {
    const size_t size = width * height * channels * sizeof(jbyte);
    auto newData = std::make_unique_for_overwrite< jbyte[] >(size);
    memcpy(newData.get(), data, size);

    auto newFrame = std::make_shared< Frame >(std::move(newData), width, height, channels);
    SPDLOG_INFO("JniCameraSink: New data width " + std::to_string(width) + " height " + std::to_string(height) + " addr " + std::to_string((unsigned long long)newFrame.get()));
    loadNewFrame(newFrame);
}
