#pragma once

#include <common/types.h>
#include <game/ruleset/ruleset_bms.h>

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace lunaticvibes::ir
{

struct ScoreResult
{
    size_t place;
    size_t total_players;
    uint8_t clear_rate; // 0-100.
};

enum class SettingType
{
    TEXT,
    PASSWORD,
};

class Client
{
private:
    struct CreationToken
    {
        explicit CreationToken() = default;
    };

public:
    [[nodiscard]] static std::optional<Client> open(const Path& p);
    [[nodiscard]] static std::vector<Path> discover();

    [[nodiscard]] const std::string& id() { return _id; };
    std::optional<ScoreResult> send_score(const std::shared_ptr<ScoreBMS>& score,
                                          const std::shared_ptr<ChartFormatBMSMeta>& song,
                                          lunaticvibes::BmsGaugeType gauge, PlayModifierRandomType mods);
    void open_ir_page(const std::string& md5);

    [[nodiscard]] std::map<std::string, SettingType> get_settings();
    void apply_settings(const std::map<std::string, std::string>&);

    Client(const Client&) = delete;
    Client(Client&&) = default;
    Client& operator=(const Client&) = delete;
    Client& operator=(Client&&) noexcept = default;
    explicit Client(std::string id, sol::state, CreationToken);

private:
    sol::state _lua;
    std::string _id;
};

} // namespace lunaticvibes::ir
