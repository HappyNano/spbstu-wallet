#pragma once

#include <memory>
#include <mutex>

#include "frame.h"

namespace cxx {

    class CameraSink {
    public:
        CameraSink() = default;
        virtual ~CameraSink() = default;

        auto getLastFrame() const -> std::shared_ptr< Frame >;
        void loadNewFrame(std::shared_ptr< Frame > newFrame);

    private:
        // TODO: atomic_shared
        std::shared_ptr< Frame > lastFrame_;
        mutable std::mutex mutex_;
    };

} // namespace cxx
