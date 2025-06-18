#include <common/assert.h>
#include <common/beat.h>
#include <common/chartformat/chartformat_bms.h>
#include <common/hash.h>
#include <common/in_test_mode.h>
#include <common/log.h>
#include <common/meta.h>
#include <common/sysutil.h>
#include <common/u8.h>
#include <common/utils.h>
#include <config/cfg_general.h>
#include <config/cfg_profile.h>
#include <config/config_mgr.h>
#include <db/db_score.h>
#include <db/db_song.h>
#include <game/arena/arena_client.h>
#include <game/arena/arena_data.h>
#include <game/arena/arena_host.h>
#include <game/graphics/graphics.h>
#include <game/input/input_mgr.h>
#include <game/runtime/generic_info.h>
#include <game/runtime/i18n.h>
#include <game/runtime/state.h>
#include <game/scene/scene.h>
#include <game/scene/scene_context.h>
#include <game/scene/scene_mgr.h>
#include <game/skin/skin_lr2.h>
#include <game/skin/skin_lr2_dst.h>
#include <game/skin/skin_lr2_slider_callbacks.h>
#include <game/skin/skin_mgr.h>
#include <game/sound/sound_mgr.h>
#include <git_version.h>

#include <format>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#pragma comment(lib, "winmm.lib")
#include <ole2.h>
#include <timeapi.h>
#endif // WIN32

#ifdef RENDER_SDL3
#include <SDL3/SDL_main.h> // WinMain implementation
#endif                     // RENDER_SDL3

#include <boost/nowide/args.hpp>

#include <curl/curl.h>
#include <imgui.h>
#include <tinyfiledialogs.h>

static constexpr auto&& IN_MEMORY_DB_PATH = ":memory:";

bool lunaticvibes::in_test_mode()
{
#ifndef NDEBUG
    return true;
#else
    return false;
#endif
}

static void mainLoop();

template <class F> struct Defer
{
    Defer(F f) : f(f) {}
    ~Defer() { f(); }
    F f;
};
template <class F> [[nodiscard]] static Defer<F> mk_defer(F f)
{
    return Defer<F>(f);
};

int main(int argc, char* argv[])
{
    auto _bb_main = mk_defer([] { StopHandleMainThreadTask(); });

    boost::nowide::args _(argc, argv); // Make arguments UTF-8 encoded on Windows in-place. For CJK file paths.

    SetThreadName("LunaticVibesF");
    SetThreadAsMainThread();

    executablePath = GetExecutablePath();
    fs::current_path(executablePath);

    lunaticvibes::InitLogger("LunaticVibesF.log");

    LOG_INFO << "Starting Lunatic Vibes F " << PROJECT_VERSION << " (" << GIT_REVISION << ")";

    ConfigMgr::init();
    ConfigMgr::load();
    ConfigMgr::selectProfile(ConfigMgr::General()->get(cfg::E_PROFILE, cfg::PROFILE_DEFAULT));
    auto _save_config_on_exit = mk_defer([] { ConfigMgr::save(); });
    lunaticvibes::SetLogLevel(static_cast<lunaticvibes::LogLevel>(ConfigMgr::General()->get(cfg::E_LOG_LEVEL, 1)));

    if (Path lr2path = convertLR2Path(ConfigMgr::General()->get(cfg::E_LR2PATH, "."), "LR2files/");
        !fs::is_directory(lr2path))
    {
        const std::string msg = std::format("LR2files directory not found! {}", lunaticvibes::s(lr2path.u8string()));
        LOG_FATAL << msg;
        tinyfd_messageBox("Fatal error", msg.c_str(), "ok", "error", 0);
        return 1;
    }

    LOG_INFO << "Initializing libcurl...";
    if (CURLcode ret = curl_global_init(CURL_GLOBAL_DEFAULT); ret != CURLE_OK)
    {
        const std::string msg =
            std::format("libcurl init error: {}", static_cast<std::underlying_type_t<decltype(ret)>>(ret));
        LOG_FATAL << msg;
        tinyfd_messageBox("Fatal error", msg.c_str(), "ok", "error", 0);
        return 1;
    }
    LOG_INFO << "libcurl version: " << curl_version();
    auto _bb_curl = mk_defer([] { curl_global_cleanup(); });

    LOG_INFO << "Initializing ImGui...";
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto _bb_imgui = mk_defer([] { ImGui::DestroyContext(); });

    LOG_INFO << "ImGui version: " << ImGui::GetVersion();

    ImGui::StyleColorsDark();
    {
        ImGuiStyle& s = ImGui::GetStyle();

        s.WindowRounding = 0.f;
        s.WindowBorderSize = 0.f;
        s.FrameRounding = 0.f;
        s.FrameBorderSize = 1.f;

        auto& c = s.Colors;
        c[ImGuiCol_WindowBg] = {0.f, 0.f, 0.f, 0.7f};

        c[ImGuiCol_FrameBg] = {0.3f, 0.3f, 0.3f, 0.6f};
        c[ImGuiCol_FrameBgHovered] = {0.6f, 0.6f, 0.6f, 0.6f};
        c[ImGuiCol_FrameBgActive] = {0.5f, 0.5f, 0.5f, 1.0f};

        c[ImGuiCol_CheckMark] = {0.f, 1.f, 0.f, 0.8f};

        c[ImGuiCol_Button] = {0.f, 0.f, 0.f, 0.6f};
        c[ImGuiCol_ButtonHovered] = {0.6f, 0.6f, 0.6f, 0.6f};
        c[ImGuiCol_ButtonActive] = {0.5f, 0.5f, 0.5f, 1.0f};

        c[ImGuiCol_Header] = {0.5f, 0.5f, 0.5f, 0.4f};
        c[ImGuiCol_HeaderHovered] = {0.6f, 0.6f, 0.6f, 0.6f};
        c[ImGuiCol_HeaderActive] = {0.5f, 0.5f, 0.5f, 1.0f};

        c[ImGuiCol_Tab] = {0.f, 0.f, 0.f, 0.6f};
        c[ImGuiCol_TabHovered] = {0.6f, 0.6f, 0.6f, 0.6f};
        c[ImGuiCol_TabActive] = {0.5f, 0.5f, 0.5f, 1.0f};
        c[ImGuiCol_TabUnfocused] = {0.f, 0.f, 0.f, 0.6f};
        c[ImGuiCol_TabUnfocusedActive] = {0.5f, 0.5f, 0.5f, 0.8f};
    }

    if (auto ginit = lunaticvibes::window::graphics_init())
        return ginit;
    auto _bbgraphics = mk_defer([] { lunaticvibes::window::graphics_free(); });

    if (auto sinit = SoundMgr::initFMOD())
        return sinit;
    SoundMgr::startUpdate();
    auto _bbsoundmgr = mk_defer([] { SoundMgr::stopUpdate(); });

    InputMgr::init();
    InputMgr::updateDevices();

    ConfigMgr::setGlobals();

    i18n::init();
    i18n::setLanguage(ConfigMgr::Profile()->get(cfg::P_LANGUAGE, "English"));

    int fontIndex = 0;
    Path imguiFontPath = getSysFontPath(nullptr, &fontIndex, i18n::getCurrentLanguage());
    if (!fs::exists(imguiFontPath))
    {
        constexpr auto&& msg = "Font file not found. Please reinstall the game.";
        LOG_FATAL << msg;
        tinyfd_messageBox("Fatal error", msg, "ok", "error", 0);
        return 1;
    }
    ImFontConfig fontConfig;
    fontConfig.FontNo = fontIndex;
    ImFontAtlas& fontAtlas = *ImGui::GetIO().Fonts;
    fontAtlas.AddFontFromFileTTF(lunaticvibes::cs(imguiFontPath.u8string()), 24, &fontConfig,
                                 fontAtlas.GetGlyphRangesChineseFull());

    SoundMgr::setVolume(SampleChannel::MASTER, static_cast<float>(State::get(IndexSlider::VOLUME_MASTER)));
    SoundMgr::setVolume(SampleChannel::KEY, static_cast<float>(State::get(IndexSlider::VOLUME_KEY)));
    SoundMgr::setVolume(SampleChannel::BGM, static_cast<float>(State::get(IndexSlider::VOLUME_BGM)));
    if (State::get(IndexSwitch::SOUND_FX0))
        SoundMgr::setDSP(static_cast<DSPType>(State::get(IndexOption::SOUND_FX0)), 0,
                         static_cast<SampleChannel>(State::get(IndexOption::SOUND_TARGET_FX0)),
                         static_cast<float>(State::get(IndexSlider::FX0_P1)),
                         static_cast<float>(State::get(IndexSlider::FX0_P2)));
    if (State::get(IndexSwitch::SOUND_FX1))
        SoundMgr::setDSP(static_cast<DSPType>(State::get(IndexOption::SOUND_FX1)), 1,
                         static_cast<SampleChannel>(State::get(IndexOption::SOUND_TARGET_FX1)),
                         static_cast<float>(State::get(IndexSlider::FX1_P1)),
                         static_cast<float>(State::get(IndexSlider::FX1_P2)));
    if (State::get(IndexSwitch::SOUND_FX2))
        SoundMgr::setDSP(static_cast<DSPType>(State::get(IndexOption::SOUND_FX2)), 2,
                         static_cast<SampleChannel>(State::get(IndexOption::SOUND_TARGET_FX2)),
                         static_cast<float>(State::get(IndexSlider::FX2_P1)),
                         static_cast<float>(State::get(IndexSlider::FX2_P2)));
    lr2skin::slider::updatePitchState(State::get(IndexNumber::PITCH));
    if (State::get(IndexSwitch::SOUND_EQ))
    {
        for (int idx = 0; idx < 7; ++idx)
        {
            int val = State::get(IndexNumber(idx + static_cast<int>(IndexNumber::EQ0)));
            SoundMgr::setEQ(static_cast<EQFreq>(idx), val);
        }
    }

    gSelectContext.scrollTimeLength = ConfigMgr::Profile()->get(cfg::P_LIST_SCROLL_TIME_INITIAL, 300);

    // load songs / tables at ScenePreSelect

    if (argc >= 2)
    {
        const Path bmsFile{argc >= 3 ? argv[2] : argv[1]};
        gPlayContext.isAuto = argc >= 3 && std::string_view{argv[1]} == "-a";
        State::set(IndexSwitch::SYSTEM_AUTOPLAY, gPlayContext.isAuto);
        LOG_INFO << "[Main] isAuto=" << gPlayContext.isAuto;

        g_pScoreDB = std::make_shared<ScoreDB>(IN_MEMORY_DB_PATH);
        g_pSongDB = std::make_shared<SongDB>(IN_MEMORY_DB_PATH);

        gNextScene = SceneType::PLAY;
        gQuitOnFinish = true;

        std::shared_ptr<ChartFormatBase> bms = ChartFormatBase::createFromFile(bmsFile, std::time(nullptr));
        if (bms == nullptr)
        {
            constexpr auto&& msg = "[Main] Invalid chart file path";
            LOG_FATAL << msg;
            tinyfd_messageBox("Fatal error", msg, "ok", "error", 0);
            return 1;
        }
        gPlayContext.mode = lunaticvibes::skinTypeForKeys(bms->gamemode);

        gChartContext.path = bmsFile;
        gChartContext.hash = md5file(bmsFile);
        gChartContext.chart = bms;
        gChartContext.chartMybest = nullptr;

        gChartContext.concurrent.sampleLoadedHash = HashMD5{};
        gChartContext.concurrent.bgaLoadedHash = HashMD5{};
        gChartContext.started = false;

        gChartContext.isDoubleBattle = false;

        gChartContext.title = bms->title;
        gChartContext.title2 = bms->title2;
        gChartContext.artist = bms->artist;
        gChartContext.artist2 = bms->artist2;
        gChartContext.genre = bms->genre;
        gChartContext.version = bms->version;
        gChartContext.level = bms->levelEstimated;

        gChartContext.minBPM = bms->minBPM;
        gChartContext.startBPM = bms->startBPM;
        gChartContext.maxBPM = bms->maxBPM;

        gChartContext.texBackbmp.setPath("");
        gChartContext.texBanner.setPath("");
        gChartContext.texStagefile.setPath("");
    }
    else
    {
        gNextScene = SceneType::PRE_SELECT;
    }

#ifdef _WIN32
    timeBeginPeriod(1);
    auto _bb_high_precision_time = mk_defer([] { timeEndPeriod(1); });

    // TODO: what is Ole needed for? If clipboard works without this, drop this #win
    const HRESULT oleInitializeResult = OleInitialize(nullptr);
    if (oleInitializeResult < 0)
    {
        LOG_ERROR << "OleInitialize Failed";
    }
    auto _bb_ole = mk_defer([oleInitializeResult] {
        if (oleInitializeResult >= 0)
            OleUninitialize();
    });
#endif

    auto _bb_arena = mk_defer([] {
        if (gArenaData.isOnline())
        {
            if (gArenaData.isClient())
            {
                if (gArenaData.isOnline())
                    g_pArenaClient->leaveLobby();
                g_pArenaClient->loopEnd();
                g_pArenaClient.reset();
            }
            if (gArenaData.isServer())
            {
                if (gArenaData.isOnline())
                    g_pArenaHost->disbandLobby();
                g_pArenaHost->loopEnd();
                g_pArenaHost.reset();
            }
        }
    });

    mainLoop();

    return 0;
}

// Fix your time step.
struct FixedUpdater
{
    constexpr explicit FixedUpdater(const lunaticvibes::Time& t) : _acc(0), _last_update_time(0) { reset(t); }

    constexpr void operator()(const lunaticvibes::Time& t, unsigned rate, auto cb)
    {
        _acc += t - _last_update_time;

        const auto dt = lunaticvibes::Time{1000 / rate};
        while (_acc >= dt)
        {
            const auto this_tick_time = t - _acc;
            cb(this_tick_time);
            _acc -= dt;
        }

        _last_update_time = t;
    }

    constexpr void reset(const lunaticvibes::Time& t)
    {
        _acc = 0;
        _last_update_time = t;
    }

    lunaticvibes::Time _acc;
    lunaticvibes::Time _last_update_time;
};

void mainLoop()
{
    FixedUpdater update_global_generic_info{lunaticvibes::Time::now()};

    SceneType currentScene = SceneType::NOT_INIT;

    FixedUpdater update_scene{lunaticvibes::Time::now()};
    FixedUpdater update_scene_customize{lunaticvibes::Time::now()};

    auto skinMgr = std::make_shared<SkinMgr>();
    std::shared_ptr<SceneBase> scene;
    std::shared_ptr<SceneBase> sceneCustomize;
    while (currentScene != SceneType::EXIT && gNextScene != SceneType::EXIT)
    {
        const auto t = lunaticvibes::Time::now();

        // Event handling
        const bool quit = lunaticvibes::window::event_handle();
        if (quit)
            gAppIsExiting = true;

        // Scene change
        auto wantSceneChange = [&]() {
            if (!gInCustomize && currentScene != gNextScene)
                return true;
            if (gInCustomize && (gCustomizeSceneChanged || sceneCustomize == nullptr))
                return true;
            return gExitingCustomize.load();
        };
        if (wantSceneChange())
        {
            LOG_DEBUG << "[Main] Changing scene";

            if (gInCustomize && sceneCustomize == nullptr)
            {
                LOG_DEBUG << "[Main] Building customize";
                sceneCustomize = lunaticvibes::buildScene(skinMgr, SceneType::CUSTOMIZE);
                LVF_ASSERT(sceneCustomize != nullptr);
                sceneCustomize->inputLoopStart();
            }

            if (gExitingCustomize && sceneCustomize)
            {
                LOG_DEBUG << "[Main] Exiting customize";
                gInCustomize = false;
                sceneCustomize->inputLoopEnd();
                sceneCustomize.reset();
                gNextScene = SceneType::SELECT;
            }

            if (!gInCustomize)
                gExitingCustomize = false;
            gCustomizeSceneChanged = false;

            if (scene)
            {
                scene->inputLoopEnd();
                scene.reset();
            }

            clearCustomDstOpt();
            currentScene = gNextScene;
            if (currentScene != SceneType::EXIT && currentScene != SceneType::NOT_INIT)
            {
                State::set(IndexTimer::SCENE_START, TIMER_NEVER);
                State::set(IndexTimer::START_INPUT, TIMER_NEVER);
                State::set(IndexTimer::_LOAD_START, TIMER_NEVER);
                State::set(IndexTimer::PLAY_READY, TIMER_NEVER);
                State::set(IndexTimer::PLAY_START, TIMER_NEVER);
                scene = lunaticvibes::buildScene(skinMgr, currentScene);
                LVF_ASSERT(scene != nullptr);
                auto t = lunaticvibes::Time::now();
                State::set(IndexTimer::SCENE_START, t.norm());
                State::set(IndexTimer::START_INPUT, t.norm() + (scene ? scene->getSkinInfo().timeIntro : 0));
                if (!gInCustomize)
                    scene->inputLoopStart();
            }
        }

        // Don't really care about catching up, just want to call this once a second.
        update_global_generic_info(t, 1, &lunaticvibes::g_update_generic_info);

        if (scene)
            scene->processInput();
        if (sceneCustomize)
            sceneCustomize->processInput();

        // draw
        {
            lunaticvibes::window::graphics_clear();
            doMainThreadTask();

            if (scene)
                update_scene(t, scene->getRate(), std::bind_front(&SceneBase::update_fixed, std::ref(*scene)));
            else
                update_scene.reset(t);

            if (sceneCustomize)
                update_scene_customize(t, sceneCustomize->getRate(),
                                       std::bind_front(&SceneBase::update_fixed, std::ref(*sceneCustomize)));
            else
                update_scene_customize.reset(t);

            if (scene)
            {
                scene->update();
                scene->draw();
            }
            if (sceneCustomize)
            {
                sceneCustomize->update();
                sceneCustomize->draw();
            }
            lunaticvibes::window::graphics_flush();
        }
        lunaticvibes::g_feed_frametime(lunaticvibes::FrameTimeType::Fps, lunaticvibes::Time::now() - t);
    }
    if (scene)
    {
        scene->inputLoopEnd();
        scene.reset();
    }
}
