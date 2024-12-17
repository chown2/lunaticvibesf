#pragma once

#include <array>
#include <list>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "scene.h"
#include "scene_context.h"
#include "scene_pre_select.h"
#include <game/lua_ir.h>

enum class eSelectState
{
    PREPARE,
    SELECT,
    WATCHING_README,
    FADEOUT,
};

class ChartFormatBase;
class EntryFolderRegular;
class SceneCustomize;

class SceneSelect : public SceneBase
{
public:
    explicit SceneSelect(const std::shared_ptr<SkinMgr>& skinMgr);
    ~SceneSelect() override;

private:
    eSelectState state;
    InputMask _inputAvailable;

    int _config_list_scroll_time_initial;
    // navigate input
    bool isHoldingUp = false;
    bool isHoldingDown = false;
    bool isScrollingByAxis = false;

    lunaticvibes::Time navigateTimestamp;

    // hold SELECT to enter version list
    bool isInVersionList = false;
    lunaticvibes::Time selectDownTimestamp;

    // preview
    std::jthread _previewLoading;
    std::jthread _previewChartLoading;
    std::shared_mutex previewMutex;
    enum
    {
        PREVIEW_START_CHART_LOADING,
        PREVIEW_LOADING_CHART,
        PREVIEW_START_SAMPLE_LOADING,
        PREVIEW_LOADING_SAMPLES,
        PREVIEW_READY,
        PREVIEW_PLAY,
        PREVIEW_FINISH,
    } previewState = PREVIEW_START_CHART_LOADING;
    bool previewStandalone = false; // true if chart has a preview sound track
    long long previewStandaloneLength = 0;
    std::shared_ptr<ChartFormatBase> previewChart = nullptr;
    std::shared_ptr<ChartObjectBase> previewChartObj = nullptr;
    std::shared_ptr<RulesetBase> previewRuleset = nullptr;
    lunaticvibes::Time previewStartTime;
    lunaticvibes::Time previewEndTime;
    std::array<size_t, 128> _bgmSampleIdxBuf{};
    std::array<size_t, 128> _keySampleIdxBuf{};

    // virtual Customize scene, customize option toggle in select scene support
    std::shared_ptr<SceneCustomize> _virtualSceneCustomize;
    std::shared_ptr<SkinMgr> _skinMgr;

    // smooth scrolling
    lunaticvibes::Time scrollButtonTimestamp;
    double scrollAccumulator = 0.0;
    double scrollAccumulatorAddUnit = 0.0;

    // F8
    std::shared_ptr<ScenePreSelect> _virtualSceneLoadSongs;
    bool refreshingSongList = false;

    // 5+7 / 6+7
    bool isHoldingK15 = false;
    bool isHoldingK16 = false;
    bool isHoldingK17 = false;
    bool isHoldingK25 = false;
    bool isHoldingK26 = false;
    bool isHoldingK27 = false;

    // 9K
    bool bindings9K = false; // yellow buttons (2, 8) navigate, red button (5) enter, blue buttons (4, 6) back

    // imgui
    bool imguiShow = false;

public:
    void closeReadme(const lunaticvibes::Time&);
    void openChartReadme(const lunaticvibes::Time&);
    void openHelpFile(const lunaticvibes::Time&, size_t idx);

protected:
    // Looper callbacks
    [[nodiscard]] bool readyToStopAsync() const override;
    void _updateAsync() override;
    void updatePrepare();
    void updateSelect();
    void updateFadeout();

    void update() override;
    [[nodiscard]] bool shouldShowImgui() const override;
    void updateImgui() override;

protected:
    // Register to InputWrapper: judge / keysound
    void inputGamePress(InputMask&, const lunaticvibes::Time&);
    void inputGameHold(InputMask&, const lunaticvibes::Time&);
    void inputGameRelease(InputMask&, const lunaticvibes::Time&);

private:
    void openReadme(const lunaticvibes::Time&, std::string_view text);

    void inputGamePressSelect(InputMask&, const lunaticvibes::Time&);
    void inputGameHoldSelect(InputMask&, const lunaticvibes::Time&);
    void inputGameReleaseSelect(InputMask&, const lunaticvibes::Time&);
    void inputGameAxisSelect(double s1, double s2, const lunaticvibes::Time&);

    void inputGamePressPanel(InputMask&, const lunaticvibes::Time&);
    void inputGameReleasePanel(InputMask&, const lunaticvibes::Time&);

    void inputGamePressReadme(InputMask&, const lunaticvibes::Time&);

private:
    void enterEntry(const eEntryType type, const lunaticvibes::Time t);

private:
    void navigateUpBy1(const lunaticvibes::Time& t);
    void navigateDownBy1(const lunaticvibes::Time& t);
    void navigateEnter(const lunaticvibes::Time& t);
    void navigateBack(const lunaticvibes::Time& t, bool sound = true);
    void decide();
    void navigateVersionEnter(const lunaticvibes::Time& t);
    void navigateVersionBack(const lunaticvibes::Time& t);
    bool closeAllPanels(const lunaticvibes::Time& t);

protected:
    bool checkAndStartTextEdit() override;
    void inputGamePressTextEdit(InputMask&, const lunaticvibes::Time&);
    void stopTextEdit(bool modify) override;
    void resetJukeboxText();
    void searchSong(const std::string& text);

protected:
    void updatePreview();
    void postStartPreview();

    /// //////////////////////////////////////////////////////
protected:
    void arenaCommand();
    void arenaHostLobby();
    void arenaLeaveLobby();
    void arenaJoinLobbyPrompt();
    void arenaJoinLobby();

    /// //////////////////////////////////////////////////////

private:
    void imguiInit();

    // imgui Dialogs
    void imguiSettings();

    // pages
    void imguiPageOptions();
    void imguiPageOptionsGeneral();
    void imguiPageOptionsIr();
    void imguiPageOptionsJukebox();
    void imguiPageOptionsVideo();
    void imguiPageOptionsAudio();
    void imguiPageOptionsPlay();
    void imguiPageOptionsSelect();
    void imguiPageDebug();
    void imguiPageDebugArena();
    void imguiPageDebugMain();
    void imguiPageAbout();
    void imguiPageExit();

    // misc
    void imguiLoadIrClientSettings(std::span<lunaticvibes::ir::Client>);
    void imguiRefreshProfileList();
    void imguiRefreshLanguageList();
    void imguiRefreshFolderList();
    void imguiRefreshTableList();
    void imguiRefreshVideoDisplayResolutionList();
    void imguiCheckSettings();

    // buttons
    bool imguiApplyPlayerName();
    bool imguiAddFolder(const char* path = nullptr);
    bool imguiDelFolder();
    bool imguiBrowseFolder();
    bool imguiAddTable();
    bool imguiDelTable();
    bool imguiApplyResolution();
    bool imguiRefreshAudioDevices();
    bool imguiReloadIr();
    bool imguiApplyIrSettings();
    bool imguiApplyAudioSettings();

    // etc
    bool imguiArenaJoinLobbyPrompt();

    // imgui variables
    int imgui_main_index = 0;

    std::list<std::string> imgui_profiles;
    std::vector<const char*> imgui_profiles_display;
    int old_profile_index, imgui_profile_index;

    bool imgui_add_profile_popup = false;
    int imgui_add_profile_popup_error = 0;
    char imgui_add_profile_buf[256] = {0};
    bool imgui_add_profile_copy_from_current = true;

    char imgui_player_name_buf[256] = {0};

    std::list<std::string> imgui_languages;
    std::vector<const char*> imgui_languages_display;
    int old_language_index, imgui_language_index;

    int imgui_log_level;

    std::list<std::string> imgui_folders;
    std::vector<const char*> imgui_folders_display;
    int imgui_folder_index;

    char imgui_folder_path_buf[256] = {0};
    char imgui_table_url_buf[256] = {0};
    std::list<std::string> imgui_tables;
    std::vector<const char*> imgui_tables_display;
    int imgui_table_index;

    std::vector<std::pair<unsigned, unsigned>> imgui_video_display_resolution_size;
    std::vector<std::string> imgui_video_display_resolution;
    std::vector<const char*> imgui_video_display_resolution_display;
    int old_video_display_resolution_index, imgui_video_display_resolution_index;

    int old_video_mode, imgui_video_mode; // 0:windowed 1:fullscreen 2:borderless
    int imgui_video_ssLevel;

    int imgui_video_vsync_index;

    int imgui_video_maxFPS;

    std::list<std::pair<int, std::string>> imgui_audio_devices;
    std::vector<std::string> imgui_audio_devices_name;
    std::vector<const char*> imgui_audio_devices_display;
    int old_audio_device_index, imgui_audio_device_index;
    bool imgui_audio_listASIODevices;
    int imgui_audio_bufferCount;
    int imgui_audio_bufferSize;

    int imgui_adv_missBGATime;
    int imgui_adv_minInputInterval;
    int imgui_play_inputPollingRate;
    int imgui_play_defaultTarget;
    int imgui_play_judgeTiming;
    bool imgui_play_lockGreenNumber;
    float imgui_play_hispeed;
    int imgui_play_greenNumber;

    int imgui_adv_scrollSpeed[2];
    int imgui_adv_newSongDuration;
    bool imgui_adv_previewDedicated;
    bool imgui_adv_previewDirect;
    int old_adv_selectKeyBindings;
    int imgui_adv_selectKeyBindings;
    bool imgui_adv_enableNewRandom;
    bool imgui_adv_enableNewGauge;
    bool imgui_adv_enableNewLaneOption;
    bool imgui_sel_onlyDisplayMainTitleOnBars;
    bool imgui_sel_disablePlaymodeAll;
    bool imgui_sel_disableDifficultyAll;
    bool imgui_sel_disablePlaymodeSingle;
    bool imgui_sel_disablePlaymodeDouble;
    bool imgui_sel_ignoreDPCharts;
    bool imgui_sel_ignore9keys;
    bool imgui_sel_ignore5keysif7keysexist;

    bool imgui_show_arenaJoinLobbyPrompt = false;
    bool imgui_arena_joinLobby = false;
    char imgui_arena_address_buf[256] = {0};

    std::array<char, 256> _lr2_db_import_path;
    std::array<char, 256> _preview_chart_10k;
    std::array<char, 256> _preview_chart_14k;
    std::array<char, 256> _preview_chart_5k;
    std::array<char, 256> _preview_chart_7k;
    std::array<char, 256> _preview_chart_9k;

    bool imgui_play_adjustHispeedWithUpDown = false;
    bool imgui_play_adjustHispeedWithSelect = false;
    bool imgui_play_adjustLanecoverWithStart67 = false;
    bool imgui_play_adjustLanecoverWithMousewheel = false;
    bool imgui_play_adjustLanecoverWithLeftRight = false;

    std::vector<std::map<std::string, lunaticvibes::ir::SettingType>> _imgui_ir_settings;
    std::vector<std::pair<std::string, bool>> _imgui_ir;
    std::vector<std::vector<std::array<char, 512>>> _imgui_ir_setting_values;
    int _imgui_ir_settings_item = 0;
    bool _imgui_ir_did_load_clients = false;
    bool _imgui_ir_dirty = false;

    bool _imgui_show_demo_window = false;

    bool _config_enable_preview_dedicated;
    bool _config_enable_preview_direct;
    bool _show_random_any;
    bool _show_random_failed;
    bool _show_random_noplay;
};

namespace lunaticvibes
{

struct BadCommand
{
};

enum class DeleteScoreResult
{
    Ok,
    Error,
};

struct HashResult
{
    // std::nullopt if error.
    std::optional<std::string> hash;
};

struct PathResult
{
    // Empty if error on unsupported.
    std::string path;
};

using SearchQueryResult =
    std::variant<std::shared_ptr<EntryFolderRegular>, DeleteScoreResult, HashResult, PathResult, BadCommand>;

std::optional<DeleteScoreResult> try_delete_score(const std::string_view query, ScoreDB& score_db,
                                                  const SelectContextParams& select_context);
[[nodiscard]] SearchQueryResult execute_search_query(SelectContextParams& select_context, SongDB& song_db,
                                                     ScoreDB& score_db, const std::string& text);

} // namespace lunaticvibes
