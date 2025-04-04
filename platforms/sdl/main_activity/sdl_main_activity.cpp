#include "sdl_main_activity.h"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

#include <fmt/base.h>
#include <spdlog/common.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

#include <stdexcept>

using namespace cxx;

SDLMainActivity::SDLMainActivity() {
    statusBarHeight_ = 0;
    init();
}

SDLMainActivity::~SDLMainActivity() {
    shutdown();
}

void SDLMainActivity::initImpl() {
    if (initialized_) {
        return;
    }

    // Init logger
    auto logger = spdlog::stdout_logger_mt("sdlmain");
#ifndef NDEBUG
    logger->set_level(spdlog::level::debug);
#else
    logger->set_level(spdlog::level::info);
#endif
    spdlog::set_default_logger(std::move(logger));

    SPDLOG_INFO("MainActivity: {}", "Initialization");

    // Setup SDL
    // [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts would likely be your SDL_AppInit() function]
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        SPDLOG_ERROR("MainActivity: SDL_Init(): {}", SDL_GetError());
        throw std::runtime_error("See logs");
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char * glslVersion = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char * glslVersion = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char * glslVersion = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char * glslVersion = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
    window_ = SDL_CreateWindow("Dear ImGui SDL3+OpenGL3 example", 1280, 720, windowFlags);
    if (!window_) {
        SPDLOG_ERROR("MainActivity: SDL_CreateWindow(): {}", SDL_GetError());
        throw std::runtime_error("See logs");
    }
    SDL_SetWindowPosition(window_, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    glContext_ = SDL_GL_CreateContext(window_);
    if (!glContext_) {
        SPDLOG_ERROR("MainActivity: SDL_GL_CreateContext(): {}", SDL_GetError());
        throw std::runtime_error("See logs");
    }

    SDL_GL_MakeCurrent(window_, glContext_);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    SDL_ShowWindow(window_);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO & io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // ImFontConfig fontCfg;
    // fontCfg.SizePixels = 22.0f;
    // io.Fonts->AddFontDefault(&fontCfg);

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window_, glContext_);
    ImGui_ImplOpenGL3_Init(glslVersion);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    // IM_ASSERT(font != nullptr);

    // ImGui::GetStyle().ScaleAllSizes(3.0f);
    SPDLOG_INFO("MainActivity: {}", "Successful initialized");
    initialized_ = true;
}

void SDLMainActivity::shutdownImpl() {
    SPDLOG_INFO("MainActivity: {}", "Shutdown");
    if (!initialized_) {
        return;
    }

    // Cleanup
    // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppQuit() function]
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(glContext_);
    SDL_DestroyWindow(window_);
    SDL_Quit();

    // Reset logger
    spdlog::shutdown();

    initialized_ = false;
}

void SDLMainActivity::setBackgroudColor(ImVec4 color) {
    backgroudColor_ = color;
}

void SDLMainActivity::mainLoopStep() {
    ImGuiIO & io = ImGui::GetIO();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    mainLoop_->draw(shared_from_this());

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    auto backgroudColor = getBackgroundColor().value_or(ImVec4(1, 1, 1, 1));
    glClearColor(backgroudColor.x, backgroudColor.y, backgroudColor.z, backgroudColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(window_);
}

void SDLMainActivity::run() {
    bool running = true;
    while (running)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        // [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) {
                running = true;
            }
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window_)) {
                running = true;
            }
        }

        // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
        if (SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        // Initiate a new frame
        mainLoopStep();
    }
}
