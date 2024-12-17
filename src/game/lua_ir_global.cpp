#include "lua_ir_global.h"

#include <common/log.h>

#include <memory>
#include <ranges>

namespace v = std::ranges::views;

namespace lunaticvibes
{

std::shared_ptr<IrContext> get_global_ir_context()
{
    static auto ctx = std::make_shared<IrContext>();
    return ctx;
}

bool IrContext::is_online()
{
    return !clients.empty();
}

std::optional<ir::ScoreResult> IrContext::send_score(const std::shared_ptr<ScoreBMS>& score,
                                                     const std::shared_ptr<ChartFormatBMSMeta>& song,
                                                     lunaticvibes::BmsGaugeType gauge, PlayModifierRandomType mods)
{
    if (clients.empty())
    {
        LOG_DEBUG << "[LuaIRGlobal] Can't send score: no clients";
        return {};
    }

    for (auto& c : clients | v::drop(1))
    {
        auto result = c.send_score(score, song, gauge, mods);
        if (!result.has_value())
            LOG_WARNING << "[LuaIRGlobal] Seemingly failed to upload score for client " << c.id();
    }

    auto result = clients.front().send_score(score, song, gauge, mods);
    if (!result.has_value())
        LOG_WARNING << "[LuaIRGlobal] Seemingly failed to upload score for (main) client " << clients.front().id();
    return result;
}

} // namespace lunaticvibes
