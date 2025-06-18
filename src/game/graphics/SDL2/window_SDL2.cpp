#ifdef RENDER_SDL3
// TODO: reduce casts to floats for SDL_FRect

#include <atomic>
#include <bit>
#include <climits>
#include <cstdint>
#include <mutex>
#include <ranges>
#include <string>
#include <utility>

#include <SDL3/SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <common/assert.h>
#include <common/encoding.h>
#include <common/log.h>
#include <common/meta.h>
#include <common/sysutil.h>
#include <common/u8.h>
#include <common/utils.h>
#include <config/cfg_general.h>
#include <config/config_mgr.h>
#include <game/graphics/SDL2/graphics_SDL2.h>
#include <game/graphics/SDL2/image_SDL2.h>
#include <game/graphics/SDL2/input_SDL2.h>
#include <game/graphics/graphics.h>
#include <game/graphics/video.h>

namespace v = std::views;

static std::atomic<double> s_canvas_scale_x = 1.0;
static std::atomic<double> s_canvas_scale_y = 1.0;
static std::atomic<int> s_canvas_rect_h;
static std::atomic<int> s_canvas_rect_w;
static std::atomic<int> s_window_rect_h;
static std::atomic<int> s_window_rect_w;

static SDL_Texture* gInternalRenderTarget;
static SDL_Window* gFrameWindow;

int lunaticvibes::window::graphics_init()
{
    LOG_INFO << "[SDL2] Initializing...";

    // SDL2
    {
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            LOG_FATAL << "[SDL2] Library init ERROR: " << SDL_GetError();
            return -99;
        }
        LOG_INFO << "[SDL2] Library version " << SDL_MAJOR_VERSION << '.' << SDL_MINOR_VERSION << "."
                 << SDL_MICRO_VERSION;

        std::string title;
        title += PROJECT_NAME;
#ifndef NDEBUG
        title += " Debug";
#endif

#ifdef _WIN32
        // I have tested several renderers on Windows and each of them has its own issues.
        // direct3d (D3D9): skin "filter" parameters are affected by textures
        // direct3d11: such a simple 2D game should not be so picky about hardware...
        // direct3d12: same as above; plus the dll from the official site does not include this renderer
        // opengl: does not support certain custom blend mode
        // opengles2: does not support virtual textures created by hand (e.g. black dot)
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11");
#else
        // SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
#endif

        SDL_SetHint(SDL_HINT_IME_IMPLEMENTED_UI, "composition");

        Uint32 flags = SDL_WINDOW_HIDDEN; // window created with opengles2 will destroy itself when creating Renderer,
                                          // resulting in flashes
        flags |= SDL_WINDOW_RESIZABLE;
        auto mode = ConfigMgr::General()->get(cfg::V_WINMODE, cfg::V_WINMODE_WINDOWED);
        if (lunaticvibes::iequals(mode, cfg::V_WINMODE_BORDERLESS))
        {
            flags |= SDL_WINDOW_BORDERLESS;
        }
        else if (lunaticvibes::iequals(mode, cfg::V_WINMODE_FULL))
        {
            flags |= SDL_WINDOW_FULLSCREEN;
        }
        else
        {
            // fallback to windowed
        }
        s_window_rect_w = ConfigMgr::General()->get(cfg::V_DISPLAY_RES_X, CANVAS_WIDTH);
        s_window_rect_h = ConfigMgr::General()->get(cfg::V_DISPLAY_RES_Y, CANVAS_HEIGHT);
        gFrameWindow = SDL_CreateWindow(title.c_str(), s_window_rect_w, s_window_rect_h, flags);
        if (!gFrameWindow)
        {
            LOG_FATAL << "[SDL2] Init window ERROR! " << SDL_GetError();
            return -1;
        }

        graphics_resize_canvas(s_window_rect_w, s_window_rect_h);

        int ss = ConfigMgr::General()->get(cfg::V_RES_SUPERSAMPLE, 1);
        graphics_set_supersample_level(ss);

        int maxFPS = ConfigMgr::General()->get(cfg::V_MAXFPS, 480);
        if (maxFPS < 30 && maxFPS != 0)
            maxFPS = 30;
        graphics_set_maxfps(maxFPS);

        gFrameRenderer = SDL_CreateRenderer(gFrameWindow, nullptr);
        if (!gFrameRenderer)
        {
            LOG_FATAL << "[SDL2] Init renderer ERROR! " << SDL_GetError();
            return -2;
        }
        SDL_SetRenderVSync(gFrameRenderer, ConfigMgr::General()->get(cfg::V_VSYNC, false) ? 1 : 0);
        SDL_SetRenderDrawColor(gFrameRenderer, 0, 0, 0, 255);

        SDL_ShowWindow(gFrameWindow);

        if (const char* name = SDL_GetRendererName(gFrameRenderer); name != nullptr)
        {
            LOG_INFO << "[SDL2] Renderer driver: " << name;
        }

#ifdef _WIN32
        setWindowHandle(
            SDL_GetPointerProperty(SDL_GetWindowProperties(gFrameWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
#endif

        gInternalRenderTarget = SDL_CreateTexture(gFrameRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET,
                                                  CANVAS_WIDTH_MAX, CANVAS_HEIGHT_MAX);
        if (!gInternalRenderTarget)
        {
            LOG_FATAL << "[SDL2] Init Target Texture Error! " << SDL_GetError();
            return -3;
        }
        // TODO: an option to set SDL_ScaleModeBest (anisotropic filtering).
        // Nearest neighbour as the default makes most sense, as that's what LR2 does (in windowed mode).
        SDL_SetTextureScaleMode(gInternalRenderTarget, SDL_SCALEMODE_NEAREST);

        SDL_SetRenderTarget(gFrameRenderer, gInternalRenderTarget);

        SDL_HideCursor();

        LOG_INFO << "[SDL2] SDL2 init finished";
    }

    // imgui
    LOG_INFO << "[SDL2] Initializing ImGui for SDL renderer...";
    if (!ImGui_ImplSDL3_InitForSDLRenderer(gFrameWindow, gFrameRenderer) ||
        !ImGui_ImplSDLRenderer3_Init(gFrameRenderer))
    {
        LOG_FATAL << "[SDL2] ImGui init failed";
        return 3;
    }
    LOG_INFO << "[SDL2] ImGui init finished";

    // Draw a black frame to prevent flashbang
    lunaticvibes::window::graphics_clear();
    lunaticvibes::window::graphics_flush();

    return 0;
}

void lunaticvibes::window::graphics_clear()
{
    SDL_RenderClear(gFrameRenderer);
}

static int maxFPS = 0;
static bool s_should_present_imgui = false;
static std::chrono::nanoseconds desiredFrameTimeBetweenFrames;
static Path screenshotPath;
void lunaticvibes::window::graphics_flush()
{
    static std::chrono::high_resolution_clock::time_point frameTimestampPrev;

    SDL_SetRenderTarget(gFrameRenderer, nullptr);
    {
        // TODO scale internal canvas
        SDL_FRect ssRect{0, 0, static_cast<float>(s_canvas_rect_w), static_cast<float>(s_canvas_rect_h)};
        int ssLevel = lunaticvibes::window::graphics_get_supersample_level();
        ssRect.w *= ssLevel;
        ssRect.h *= ssLevel;

        const SDL_FRect windowRect{0, 0, static_cast<float>(s_window_rect_w), static_cast<float>(s_window_rect_h)};

        // render internal canvas texture
        SDL_RenderTexture(gFrameRenderer, gInternalRenderTarget, &ssRect, &windowRect);

        if (s_should_present_imgui)
        {
            if (auto* pData = ImGui::GetDrawData(); pData != nullptr)
                ImGui_ImplSDLRenderer3_RenderDrawData(pData, gFrameRenderer);
            s_should_present_imgui = false;
        }

        // Save screenshot before presenting per SDL_RenderReadPixels docs.
        if (!screenshotPath.empty())
        {
            fs::create_directories(screenshotPath.parent_path());
            SDL_Rect rect{0, 0, s_window_rect_w, s_window_rect_h};
            SDL_Surface* sshot = SDL_RenderReadPixels(gFrameRenderer, &rect);
            if (sshot == nullptr)
            {
                LOG_ERROR << "[SDL2] SDL_RenderReadPixels error: " << SDL_GetError();
            }
            else
            {
                // TODO: PNG encoding could be done on separate thread.
                (void)lunaticvibes::save_into_png(sshot, screenshotPath);
                SDL_DestroySurface(sshot);
                screenshotPath.clear();
            }
        }
        SDL_RenderPresent(gFrameRenderer);
    }
    SDL_SetRenderTarget(gFrameRenderer, gInternalRenderTarget);

    if (maxFPS != 0)
    {
        long long sleep_ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(frameTimestampPrev + desiredFrameTimeBetweenFrames -
                                                                 std::chrono::high_resolution_clock::now())
                .count();
        preciseSleep(sleep_ns);
    }
    frameTimestampPrev = std::chrono::high_resolution_clock::now();
}

int lunaticvibes::window::graphics_free()
{
    LOG_INFO << "[SDL2] Shutting down ImGui module...";
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();

    LOG_INFO << "[SDL2] Releasing SDL resources...";
    SDL_DestroyTexture(gInternalRenderTarget);
    gInternalRenderTarget = nullptr;
    SDL_DestroyRenderer(gFrameRenderer);
    gFrameRenderer = nullptr;
    SDL_DestroyWindow(gFrameWindow);
    gFrameWindow = nullptr;

    LOG_INFO << "[SDL2] De-initializing SDL2...";
    SDL_Quit();

    LOG_INFO << "[SDL2] Graphics shutdown complete.";

    return 0;
}

void lunaticvibes::window::graphics_copy_screen_texture(Texture& texture)
{
    LVF_DEBUG_ASSERT(IsMainThread());

    SDL_SetRenderTarget(gFrameRenderer, static_cast<SDL_Texture*>(texture.raw()));
    SDL_RenderClear(gFrameRenderer);
    const auto origRect = std::bit_cast<SDL_Rect>(texture.getRect());
    const auto scale = lunaticvibes::window::graphics_get_supersample_level();
    const SDL_FRect rect{0, 0, static_cast<float>(origRect.w * scale), static_cast<float>(origRect.h * scale)};
    const SDL_FRect canvasRect{0, 0, static_cast<float>(s_canvas_rect_w), static_cast<float>(s_canvas_rect_h)};
    SDL_RenderTexture(gFrameRenderer, gInternalRenderTarget, &rect, &canvasRect);
    SDL_SetRenderTarget(gFrameRenderer, gInternalRenderTarget);
}

std::pair<int, int> lunaticvibes::window::graphics_get_desktop_resolution()
{
    LVF_DEBUG_ASSERT(IsMainThread());
    SDL_DisplayID index = SDL_GetDisplayForWindow(gFrameWindow);
    const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(index);
    return {mode->w, mode->h};
}

std::vector<std::pair<int, int>> lunaticvibes::window::graphics_get_resolution_list()
{
    std::vector<std::pair<int, int>> res;

    SDL_DisplayID display = SDL_GetDisplayForWindow(gFrameWindow);
    int num_modes = 0;
    if (SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes(display, &num_modes); modes != nullptr)
    {
        for (int i = 0; i < num_modes; ++i)
        {
            SDL_DisplayMode* mode = modes[i];
            LOG_DEBUG << "[SDL2] Found resolution " << mode->w << 'x' << mode->h << ' ' << mode->refresh_rate << "Hz";
            res.emplace_back(mode->w, mode->h);
        }
        SDL_free(static_cast<void*>(modes));
    };
    return res;
}

void lunaticvibes::window::graphics_change_window_mode(lunaticvibes::GRAPHICS_WINDOW_MODE mode)
{
    LOG_INFO << "[SDL2] Setting window mode to " << static_cast<int>(mode);
    switch (mode)
    {
    case GRAPHICS_WINDOW_MODE::WINDOWED:
        SDL_SetWindowBordered(gFrameWindow, true);
        SDL_SetWindowFullscreen(gFrameWindow, false);
        return;
    case GRAPHICS_WINDOW_MODE::FAKE_FULLSCREEN:
    case GRAPHICS_WINDOW_MODE::BORDERLESS:
        SDL_SetWindowBordered(gFrameWindow, false);
        SDL_SetWindowFullscreen(gFrameWindow, false);
        return;
    case GRAPHICS_WINDOW_MODE::FULLSCREEN:
        SDL_SetWindowBordered(gFrameWindow, false);
        SDL_SetWindowFullscreen(gFrameWindow, true);
        return;
    }
    lunaticvibes::assert_failed("graphics_change_window_mode");
}

void lunaticvibes::window::graphics_resize_window(int x, int y)
{
    LOG_INFO << "[SDL2] Resizing window mode to " << x << 'x' << y;
    lunaticvibes::window::save_new_window_size(x, y);
    SDL_SetWindowSize(gFrameWindow, s_window_rect_w, s_window_rect_h);
    SDL_SetWindowPosition(gFrameWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void lunaticvibes::window::save_new_window_size(int width, int height)
{
    LOG_DEBUG << "[SDL2] Saving new window size " << width << 'x' << height;
    s_window_rect_w = width;
    s_window_rect_h = height;
    s_canvas_scale_x = static_cast<double>(width) / s_canvas_rect_w;
    s_canvas_scale_y = static_cast<double>(height) / s_canvas_rect_h;
    ConfigMgr::General()->set(cfg::V_DISPLAY_RES_X, width);
    ConfigMgr::General()->set(cfg::V_DISPLAY_RES_Y, height);
}

void lunaticvibes::window::graphics_change_vsync(bool on)
{
    LOG_INFO << "[SDL2] Setting vsync mode to " << on;
    SDL_SetRenderVSync(gFrameRenderer, on ? 1 : 0);
}

static int superSampleLevel = 1;
void lunaticvibes::window::graphics_set_supersample_level(int level)
{
    LOG_INFO << "[SDL2] Setting supersample level to " << level;
    superSampleLevel = level;
}
int lunaticvibes::window::graphics_get_supersample_level()
{
    return superSampleLevel;
}

void lunaticvibes::window::graphics_resize_canvas(int x, int y)
{
    LOG_INFO << "[SDL2] Resizing canvas to " << x << 'x' << y;
    s_canvas_rect_w = x;
    s_canvas_rect_h = y;
    s_canvas_scale_x = static_cast<double>(s_window_rect_w.load()) / x;
    s_canvas_scale_y = static_cast<double>(s_window_rect_h.load()) / y;
}
std::pair<double, double> lunaticvibes::window::graphics_get_canvas_scale()
{
    return {s_canvas_scale_x, s_canvas_scale_y};
}

void lunaticvibes::window::graphics_set_maxfps(int fps)
{
    LOG_INFO << "[SDL2] Setting max fps to " << fps;
    maxFPS = fps;
    if (maxFPS != 0)
    {
        desiredFrameTimeBetweenFrames = std::chrono::nanoseconds(static_cast<long long>(std::round(1e9 / maxFPS)));
    }
}

///////////////////////////////////////////////////////////////////////////

static bool isEditing = false;
static std::string textBuf, textBufSuffix;
static void funEditing(const SDL_TextEditingEvent& e);
static void funInput(const SDL_TextInputEvent& e);
static void funKeyDown(const SDL_KeyboardEvent& e);

bool lunaticvibes::window::event_handle()
{
    bool quit = false;
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        ImGui_ImplSDL3_ProcessEvent(&e);

        switch (e.type)
        {
        case SDL_EVENT_QUIT:
            LOG_INFO << "[SDL2] Got SDL_EVENT_QUIT";
            quit = true;
            break;

        case SDL_EVENT_WINDOW_FOCUS_GAINED: SetWindowForeground(true); break;
        case SDL_EVENT_WINDOW_FOCUS_LOST: SetWindowForeground(false); break;
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        case SDL_EVENT_WINDOW_RESIZED: {
            const auto height = e.window.data2;
            const auto width = e.window.data1;
            lunaticvibes::window::save_new_window_size(width, height);
            break;
        }

        case SDL_EVENT_TEXT_INPUT:
            if (isEditing)
                funInput(e.text);
            break;

        case SDL_EVENT_TEXT_EDITING:
            if (isEditing)
                funEditing(e.edit);
            break;

        case SDL_EVENT_KEY_DOWN: {
            std::unique_lock l{sdl::state::g_input_mutex};
            sdl::state::g_keyboard_scancodes[e.key.scancode] = true;
            l.unlock();
            if (isEditing)
                funKeyDown(e.key);
            break;
        }

        case SDL_EVENT_KEY_UP: {
            std::unique_lock l{sdl::state::g_input_mutex};
            sdl::state::g_keyboard_scancodes[e.key.scancode] = false;
            break;
        }

        case SDL_EVENT_MOUSE_MOTION: {
            std::unique_lock l{sdl::state::g_input_mutex};
            sdl::state::g_mouse_x = static_cast<int>(e.motion.x);
            sdl::state::g_mouse_y = static_cast<int>(e.motion.y);
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            std::unique_lock l{sdl::state::g_input_mutex};
            sdl::state::g_mouse_buttons[e.button.button] = true;
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_UP: {
            std::unique_lock l{sdl::state::g_input_mutex};
            sdl::state::g_mouse_buttons[e.button.button] = false;
            break;
        }

        case SDL_EVENT_MOUSE_WHEEL: {
            std::unique_lock l{sdl::state::g_input_mutex};
            sdl::state::g_mouse_wheel_delta = static_cast<std::int16_t>(e.wheel.y); // -1/1
            break;
        }

        case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
            LVF_DEBUG_ASSERT(e.gaxis.which < static_cast<int>(sdl::state::g_con_axes.size()));
            auto& axes = sdl::state::g_con_axes[e.gaxis.which];
            if (e.gaxis.axis >= axes.size())
            {
                LOG_WARNING << "[SDL2] Controller axis is out of range";
                break;
            }
            std::unique_lock l{sdl::state::g_input_mutex};
            axes[e.gaxis.axis] = e.gaxis.value;
            break;
        }

        case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
            LVF_DEBUG_ASSERT(e.gbutton.which < static_cast<int>(sdl::state::g_con_buttons.size()));
            auto& buttons = sdl::state::g_con_buttons[e.gbutton.which];
            if (e.gbutton.button >= buttons.size())
            {
                LOG_WARNING << "[SDL2] Controller button is out of range";
                break;
            }
            std::unique_lock l{sdl::state::g_input_mutex};
            buttons[e.gbutton.button] = true;
            break;
        }

        case SDL_EVENT_GAMEPAD_BUTTON_UP: {
            LVF_DEBUG_ASSERT(e.gbutton.which < static_cast<int>(sdl::state::g_con_buttons.size()));
            auto& buttons = sdl::state::g_con_buttons[e.gbutton.which];
            if (e.gbutton.button >= buttons.size())
            {
                LOG_WARNING << "[SDL2] Controller button is out of range";
                break;
            }
            std::unique_lock l{sdl::state::g_input_mutex};
            buttons[e.gbutton.button] = false;
            break;
        }

        default: break;
        }
    }
    return quit;
}

void lunaticvibes::window::ImGuiNewFrame()
{
    SDL_SetRenderTarget(gFrameRenderer, nullptr);
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    SDL_SetRenderTarget(gFrameRenderer, gInternalRenderTarget);
    ImGui::SetMouseCursor(ImGuiMouseCursor_None);
    s_should_present_imgui = true;
}

static std::function<void(const std::string&)> funUpdateText;
void lunaticvibes::window::startTextInput(const RectF& textBox, const std::string& oldText,
                                          std::function<void(const std::string&)> funUpdateText)
{
    LOG_DEBUG << "Start Text Input";

    textBuf = oldText;
    textBuf.reserve(32);

    ::funUpdateText = std::move(funUpdateText);

    SDL_Rect r;
    const RectF scaledTextBox{
        static_cast<float>(textBox.x * s_canvas_scale_x),
        static_cast<float>(textBox.y * s_canvas_scale_y),
        static_cast<float>(textBox.w * s_canvas_scale_x),
        static_cast<float>(textBox.h * s_canvas_scale_y),
    };
    r.x = static_cast<int>(std::floor(scaledTextBox.x));
    r.y = static_cast<int>(std::floor(scaledTextBox.y));
    r.w = static_cast<int>(std::ceil(scaledTextBox.w));
    r.h = static_cast<int>(std::ceil(scaledTextBox.h));
    SDL_SetTextInputArea(gFrameWindow, &r, 0);
    SDL_StartTextInput(gFrameWindow);
    isEditing = true;

    ::funUpdateText(textBuf);
}

void lunaticvibes::window::stopTextInput()
{
    LOG_DEBUG << "Stop Text Input";

    isEditing = false;
    funUpdateText = [](const std::string&) {};
    SDL_StopTextInput(gFrameWindow);
}

void funEditing(const SDL_TextEditingEvent& e)
{
    LOG_DEBUG << "Editing " << e.start << " " << e.length << " " << e.text;
    if (e.length == 0)
    {
        textBuf += textBufSuffix;
        textBufSuffix.clear();
        funUpdateText(textBuf);
    }
    else
    {
        funUpdateText(textBuf + e.text + textBufSuffix);
    }
}

void funInput(const SDL_TextInputEvent& e)
{
    textBuf = textBuf + e.text + textBufSuffix;
    textBufSuffix.clear();
    funUpdateText(textBuf);
}

// \param c First byte of UTF-8 character (which may be up to 4 bytes)
static int get_utf8_char_size(char c)
{
    if ((c & 0x80) == 0)
        return 1;
    if ((c & 0xE0) == 0xC0)
        return 2;
    if ((c & 0xF0) == 0xE0)
        return 3;
    if ((c & 0xF8) == 0xF0)
        return 4;
    return -1;
}

static void remove_last_utf8_char(std::string& str)
{
    if (str.empty())
        return;
    const auto s_size = static_cast<int>(str.size());
    for (const int size :
         str | v::reverse | v::transform(get_utf8_char_size) | v::filter([](int size) { return size > 0; }))
    {
        str.resize(s_size < size ? 0 : s_size - size);
        break;
    }
}

void funKeyDown(const SDL_KeyboardEvent& e)
{
    if (e.mod & (SDL_KMOD_LCTRL | SDL_KMOD_RCTRL))
    {
        switch (e.key)
        {
        case SDLK_V: {
            std::stringstream ss;
            char* pText = SDL_GetClipboardText();
            ss << pText;
            SDL_free(pText);
            std::string textLine;
            std::getline(ss, textLine);
            textBuf += textLine;
            funUpdateText(textBuf + textBufSuffix);
            break;
        }
        default: break;
        }
    }
    else
    {
        switch (e.key)
        {
        case SDLK_BACKSPACE:
            if (textBufSuffix.empty() && !textBuf.empty())
            {
                remove_last_utf8_char(textBuf);
                funUpdateText(textBuf + textBufSuffix);
            }
            break;
        default: break;
        }
    }
}

#endif

void lunaticvibes::window::queue_screenshot(Path png)
{
    LOG_INFO << "[SDL2] Screenshot: " << png;
    screenshotPath = std::move(png);
}

std::pair<int, int> lunaticvibes::window::get_mouse_pos()
{
    std::shared_lock l{sdl::state::g_input_mutex};
    return {sdl::state::g_mouse_x, sdl::state::g_mouse_y};
}
