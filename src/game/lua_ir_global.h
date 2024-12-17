#pragma once

#include <common/chartformat/chartformat_bms.h>
#include <common/types.h>
#include <game/lua_ir.h>
#include <game/ruleset/ruleset_bms.h>

#include <vector>

namespace lunaticvibes
{

struct IrContext
{
    std::vector<lunaticvibes::ir::Client> clients;

    [[nodiscard]] bool is_online();
    std::optional<ir::ScoreResult> send_score(const std::shared_ptr<ScoreBMS>& score,
                                              const std::shared_ptr<ChartFormatBMSMeta>& song,
                                              lunaticvibes::BmsGaugeType gauge, PlayModifierRandomType mods);
};

std::shared_ptr<IrContext> get_global_ir_context();

} // namespace lunaticvibes
