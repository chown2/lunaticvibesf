#pragma once

#include "ruleset_bms.h"
#include "game/scene/scene_context.h"

class RulesetBMSReplay : public RulesetBMS
{
public:
    RulesetBMSReplay(
        std::shared_ptr<ChartFormatBase> format,
        std::shared_ptr<ChartObjectBase> chart,
        std::shared_ptr<ReplayChart> replay,
        PlayModifierGaugeType gauge,
        GameModeKeys keys,
        JudgeDifficulty difficulty = JudgeDifficulty::NORMAL,
        double health = 1.0,
        PlaySide side = PlaySide::AUTO);

public:
    double playbackSpeed = 1.0;

protected:
    std::shared_ptr<ReplayChart> replay;
    std::vector<ReplayChart::Commands>::iterator itReplayCommand;
    InputMask keyPressing;
    unsigned replayCmdMapIndex = 0;

public:

    // Register to InputWrapper
    void updatePress(InputMask& pg, const lunaticvibes::Time& t) override {}
    // Register to InputWrapper
    void updateHold(InputMask& hg, const lunaticvibes::Time& t) override {}
    // Register to InputWrapper
    void updateRelease(InputMask& rg, const lunaticvibes::Time& t) override {}
    // Register to InputWrapper
    void updateAxis(double s1, double s2, const lunaticvibes::Time& t) override {}
    // Called by ScenePlay
    void update(const lunaticvibes::Time& t) override;

    void fail() override;
};