#include "lua_ir.h"

#include <common/assert.h>
#include <common/chartformat/chartformat_bms.h>
#include <common/log.h>
#include <common/sysutil.h>
#include <common/types.h>
#include <common/u8.h>
#include <game/ruleset/ruleset_bms.h>
#include <game/scene/scene_context.h>

#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <curl/curl.h>

struct CurlDeleter
{
    void operator()(CURL* curl) { curl_easy_cleanup(curl); }
};

static size_t write_to_string(const void* ptr, size_t size, size_t n, void* user_pointer)
{
    LVF_DEBUG_ASSERT(user_pointer != nullptr);
    std::string& user_data = *static_cast<std::string*>(user_pointer);
    user_data.append(static_cast<const char*>(ptr), size * n);
    return size * n;
}

static std::string send_post(const std::string& url, const std::string& s)
{
    CURLcode res;

    std::unique_ptr<CURL, CurlDeleter> curl{curl_easy_init()};

    res = curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    LVF_VERIFY(res == CURLE_OK);

    std::string answer;

    res = curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 15);
    LVF_VERIFY(res == CURLE_OK);

    res = curl_easy_setopt(curl.get(), CURLOPT_CUSTOMREQUEST, "POST");
    LVF_VERIFY(res == CURLE_OK);

    res = curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, s.data());
    LVF_VERIFY(res == CURLE_OK);
    res = curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE, s.size());
    LVF_VERIFY(res == CURLE_OK);

    res = curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_to_string);
    LVF_VERIFY(res == CURLE_OK);
    res = curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, reinterpret_cast<void*>(&answer));
    LVF_VERIFY(res == CURLE_OK);

    res = curl_easy_perform(curl.get());
    if (res != CURLE_OK)
    {
        LOG_WARNING << "[LuaIR] Send post error: " << curl_easy_strerror(res);
        return "(yabai)";
    }

    return answer;
}

static void notify(const std::string& s)
{
    createNotification(s);
}

static constexpr int LVF_IR_CLIENT_API_VERSION = -1;

[[nodiscard]] static std::optional<sol::state> open_ir_state(const fs::path& p)
try
{
    LOG_VERBOSE << "[LuaIR] IR client path: '" << p << "'";
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package);
    lua.open_libraries(sol::lib::base, sol::lib::string);

    auto lvf = lua["lvf"].get_or_create<sol::table>();
    lvf["md5"].set_function([](const std::string_view s) { return md5(s).hexdigest(); });
    lvf["notify"].set_function(&notify);
    lvf["open_uri"].set_function(&lunaticvibes::open);
    lvf["send_post"].set_function(&send_post);

    auto res = lua.script_file(p.native());
    if (!res.valid())
    {
        LOG_WARNING << "[LuaIR] Failed to source IR script";
        return {};
    }
    if (res.get_type() != sol::type::table)
    {
        LOG_WARNING << "[LuaIR] IR script doesn't return a table";
        return {};
    }

    sol::table t = res;
    lua["za_ir"] = t;

    int api_version = t.get_or("api_version", -1234);
    if (api_version != LVF_IR_CLIENT_API_VERSION)
    {
        LOG_WARNING << "[LuaIR] Unsupported IR client API version: " << api_version;
        return {};
    }

    std::string name = t.get_or("name", std::string{"wtf"});
    if (name == "wtf")
    {
        LOG_WARNING << "[LuaIR] IR client missing 'name'";
        return {};
    }
    LOG_VERBOSE << "[LuaIR] Opening IR client: " << name;

    auto check_method = [](const sol::table& t, const char* name) {
        sol::protected_function func = t[name];
        if (!func.valid())
        {
            LOG_WARNING << "[LuaIR] IR client missing method '" << name << "'";
            return false;
        }
        return true;
    };

    if (!check_method(t, "open_ir_page"))
        return {};
    if (!check_method(t, "send_score"))
        return {};

    if (!check_method(t, "get_settings"))
        return {};
    if (!check_method(t, "apply_settings"))
        return {};

    LOG_DEBUG << "[LuaIR] Opened IR client: " << name;

    return lua;
}
catch (const std::exception& e)
{
    LOG_ERROR << "[LuaIR] IR client failed ('" << p << "'): " << e.what();
    return {};
}

[[nodiscard]] static const char* gauge_to_lua_string(lunaticvibes::BmsGaugeType type)
{
    switch (type)
    {
    case lunaticvibes::BmsGaugeType::GROOVE: return "GROOVE";
    case lunaticvibes::BmsGaugeType::EASY: return "EASY";
    case lunaticvibes::BmsGaugeType::ASSIST: return "ASSIST";
    case lunaticvibes::BmsGaugeType::HARD: return "HARD";
    case lunaticvibes::BmsGaugeType::EXHARD: return "EXHARD";
    case lunaticvibes::BmsGaugeType::DEATH: return "DEATH";
    case lunaticvibes::BmsGaugeType::P_ATK: lunaticvibes::verify_failed("gauge_to_lua_string"); return "PATTACK";
    case lunaticvibes::BmsGaugeType::G_ATK: lunaticvibes::verify_failed("gauge_to_lua_string"); return "GATTACK";
    case lunaticvibes::BmsGaugeType::GRADE: return "GRADE";
    case lunaticvibes::BmsGaugeType::EXGRADE: return "EXGRADE";
    }
    lunaticvibes::assert_failed("gauge_to_lua_string");
}

[[nodiscard]] static const char* mod_to_lua_string(PlayModifierRandomType type)
{
    switch (type)
    {
    case PlayModifierRandomType::NONE: return "NONE";
    case PlayModifierRandomType::MIRROR: return "MIRROR";
    case PlayModifierRandomType::RANDOM: return "RANDOM";
    case PlayModifierRandomType::SRAN: return "S-RANDOM";
    case PlayModifierRandomType::HRAN: return "H-RANDOM";
    case PlayModifierRandomType::ALLSCR: return "ALL-SCR";
    case PlayModifierRandomType::RRAN:
    case PlayModifierRandomType::DB_SYNCHRONIZE:
    case PlayModifierRandomType::DB_SYMMETRY: lunaticvibes::verify_failed("mod_to_lua_string"); return "(ERROR)";
    }
    lunaticvibes::assert_failed("mod_to_lua_string");
}

[[nodiscard]] static const char* lamp_to_lua_string(ScoreBMS::Lamp type)
{
    switch (type)
    {
    case ScoreBMS::Lamp::NOPLAY: return "NOPLAY";
    case ScoreBMS::Lamp::FAILED: return "FAILED";
    case ScoreBMS::Lamp::ASSIST: return "ASSIST";
    case ScoreBMS::Lamp::EASY: return "EASY";
    case ScoreBMS::Lamp::NORMAL: return "NORMAL";
    case ScoreBMS::Lamp::HARD: return "HARD";
    case ScoreBMS::Lamp::EXHARD: return "EXHARD";
    case ScoreBMS::Lamp::FULLCOMBO: return "FULLCOMBO";
    case ScoreBMS::Lamp::PERFECT: return "PERFECT";
    case ScoreBMS::Lamp::MAX: return "MAX";
    }
    lunaticvibes::assert_failed("lamp_to_lua_string");
}

namespace lunaticvibes::ir
{

std::optional<Client> Client::open(const Path& p)
{
    try
    {
        if (auto ir = open_ir_state(p); ir.has_value())
            return Client{lunaticvibes::u8str(p), std::move(*ir), CreationToken{}};
    }
    catch (const std::exception& e)
    {
        LOG_ERROR << "[LuaIR] Error while opening IR client: " << e.what();
    }
    LOG_WARNING << "[LuaIR] Opening IR client failed";
    return {};
}

std::vector<Path> Client::discover()
try
{
    std::vector<Path> out;
    for (const auto& p : fs::directory_iterator("./gamedata/resources/test_ir_clients"))
    {
        if (!p.is_regular_file() || p.path().filename().extension() != ".lua" ||
            lunaticvibes::u8str(p.path().filename()).starts_with("_"))
            continue;
        out.push_back(p.path());
    }
    return out;
}
catch (const std::filesystem::filesystem_error& e)
{
    LOG_WARNING << "[LuaIR] Failed to discover IR clients: " << e.what();
    return {};
}

std::map<std::string, SettingType> Client::get_settings()
{
    LOG_VERBOSE << "[LuaIR] Getting settings";
    sol::protected_function get_settings = _lua["za_ir"]["get_settings"];
    const auto result = get_settings();
    if (!result.valid() || result.get_type() != sol::type::table)
    {
        LOG_WARNING << "[LuaIR] Getting settings failed";
        return {};
    }
    const sol::table t = result;
    std::map<std::string, SettingType> out;
    for (const auto& [k, v] : t)
    {
        auto vv = v.as<std::string>();
        SettingType type;
        if (vv == "TEXT")
        {
            type = SettingType::TEXT;
        }
        else if (vv == "PASSWORD")
        {
            type = SettingType::PASSWORD;
        }
        else
        {
            LOG_WARNING << "[LuaIR] Invalid setting type: " << vv;
            continue;
        }
        out[k.as<std::string>()] = type;
    }
    return out;
}

void Client::apply_settings(const std::map<std::string, std::string>& settings)
{
    LOG_VERBOSE << "[LuaIR] Applying settings";
    sol::protected_function apply_settings = _lua["za_ir"]["apply_settings"];
    apply_settings(settings);
}

std::optional<ScoreResult> Client::send_score(const std::shared_ptr<ScoreBMS>& score,
                                              const std::shared_ptr<ChartFormatBMSMeta>& song,
                                              lunaticvibes::BmsGaugeType gauge, PlayModifierRandomType mods)
{
    LOG_DEBUG << "[LuaIR] Uploading score to IR " << _id;

    sol::table t = _lua["za_ir"];

    auto lua_score = _lua["forzemfunctions"].get_or_create<sol::table>();

    lua_score["md5"] = gChartContext.hash.hexdigest();

    lua_score["lamp"] = lamp_to_lua_string(score->lamp);

    // If /irupdateall only sends PBs, these shouldn't be sent.
    LVF_VERIFY(score->clearcount == 0 || score->clearcount == 1);
    lua_score["play_gauge"] = gauge_to_lua_string(gauge);
    // FIXME: autoscratch type
    lua_score["play_mods"] = mod_to_lua_string(mods);
    auto lua_graph = lua_score["gauge_graph"].get_or_create<sol::table>();
    for (size_t i = 0; i < 500; ++i) // FIXME: stub
        lua_graph.add(100);

    // Song info like in LR2IR
    // FIXME: test and add to meta.lua
    lua_score["totalnotes"] = song->totalNotes;
    lua_score["judge"] = song->raw_rank;    // aka 'judge rank'
    lua_score["gamemode"] = song->gamemode; // aka 'line'
    lua_score["title"] = song->title;       // FIXME: what about title2 aka subtitle?
    lua_score["genre"] = song->genre;
    lua_score["artist"] = song->artist; // FIXME: what about artist2 aka subartist?
    lua_score["maxbpm"] = song->maxBPM;
    lua_score["minbpm"] = song->minBPM;
    lua_score["playlevel"] = song->playLevel;

    lua_score["exscore"] = score->exscore;
    lua_score["maxcombo"] = score->maxcombo;
    lua_score["minbp"] = score->bp;
    lua_score["moneyscore"] = score->score;
    lua_score["notesplayed"] = score->notes; // FIXME: score->notes is the same as song->totalNotes

    // FIXME: verify these mean what I think they do.
    lua_score["pgreat"] = score->pgreat;
    lua_score["great"] = score->great;
    lua_score["good"] = score->good;
    lua_score["bad"] = score->bad;
    lua_score["poor"] = score->kpoor;
    lua_score["miss"] = score->miss;

    sol::protected_function send_score = t["send_score"];
    const auto result = send_score(lua_score);
    if (!result.valid() || result.get_type() != sol::type::table)
    {
        LOG_DEBUG << "[LuaIR] Score uploading failed";
        return {};
    }

    const auto tt = result.get<sol::table>();

    const size_t place = tt.get_or("place", 99999);
    const size_t total_players = tt.get_or("total_players", 99999);
    const uint8_t clear_rate = tt.get_or("clar_rate", 100);

    LOG_DEBUG << "[LuaIR] IR " << _id << ": " << place << "/" << total_players << " (rate "
              << static_cast<int>(clear_rate) << ")";

    return ScoreResult{place, total_players, clear_rate};
}

void Client::open_ir_page(const std::string& md5)
{
    std::function<void(const std::string&)> open_ir_page = _lua["za_ir"]["open_ir_page"];
    open_ir_page(md5);
}

Client::Client(std::string id, sol::state state, CreationToken) : _id(std::move(id)), _lua(std::move(state)) {}

} // namespace lunaticvibes::ir
