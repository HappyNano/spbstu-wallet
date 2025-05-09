#pragma once

#include <SDL3/SDL.h>
#include <imgui.h>

#include <platforms/common/main_activity/i_main_activity.h>

#include <memory>

namespace cxx {
    class SDLMainActivity final: public IMainActivity, public std::enable_shared_from_this< SDLMainActivity > {
    public:
        SDLMainActivity();
        ~SDLMainActivity() override;

        void mainLoopStep(); // override;
        // IMainActivity
        void run() override;

    public:
        void setBackgroudColor(ImVec4 color);

    private:
        // IMainActivity
        void initImpl() override;
        void shutdownImpl() override;

    private:
        SDL_Window * window_ = nullptr;
        SDL_GLContext glContext_ = nullptr;
    };
} // namespace cxx
