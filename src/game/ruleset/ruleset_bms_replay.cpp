#include "ruleset_bms_replay.h"
#include "game/scene/scene.h"
#include "game/scene/scene_context.h"

#include <cassert>
#include <optional>
#include <utility>

RulesetBMSReplay::RulesetBMSReplay(
    std::shared_ptr<ChartFormatBase> format_,
    std::shared_ptr<ChartObjectBase> chart_,
    std::shared_ptr<ReplayChart> replay_,
    PlayModifierGaugeType gauge,
    GameModeKeys keys,
    JudgeDifficulty difficulty,
    double health,
    PlaySide side)
    : RulesetBase(format_, chart_)
    , RulesetBMS(std::move(format_), std::move(chart_), gauge, keys, difficulty, health, side)
    , replay(std::move(replay_))
{
    itReplayCommand = replay->commands.begin();
    showJudge = (_side == PlaySide::AUTO || _side == PlaySide::AUTO_DOUBLE || _side == PlaySide::AUTO_2P);

    doJudge = false;

    if (gPlayContext.mode == SkinType::PLAY5 || gPlayContext.mode == SkinType::PLAY5_2)
    {
        if (gPlayContext.shift1PNotes5KFor7KSkin)
        {
            replayCmdMapIndex = gPlayContext.shift2PNotes5KFor7KSkin ? 3 : 2;
        }
        else
        {
            replayCmdMapIndex = gPlayContext.shift2PNotes5KFor7KSkin ? 1 : 0;
        }
    }

    if (replay->pitchValue != 0)
    {
        static const double tick = std::pow(2, 1.0 / 12);
        playbackSpeed = std::pow(tick, replay->pitchValue);
    }

    switch (side)
    {
    case RulesetBMS::PlaySide::AUTO:
    case RulesetBMS::PlaySide::AUTO_DOUBLE:
        _judgeScratch = !(gPlayContext.mods[PLAYER_SLOT_PLAYER].assist_mask & PLAY_MOD_ASSIST_AUTOSCR);
        break;

    case RulesetBMS::PlaySide::AUTO_2P:
        _judgeScratch = !(gPlayContext.mods[PLAYER_SLOT_TARGET].assist_mask & PLAY_MOD_ASSIST_AUTOSCR);
        break;

    case RulesetBMS::PlaySide::MYBEST:
        _judgeScratch = !(gPlayContext.mods[PLAYER_SLOT_MYBEST].assist_mask & PLAY_MOD_ASSIST_AUTOSCR);
        break;

    case RulesetBMS::PlaySide::SINGLE:
    case RulesetBMS::PlaySide::DOUBLE:
    case RulesetBMS::PlaySide::BATTLE_1P:
    case RulesetBMS::PlaySide::BATTLE_2P:
    case RulesetBMS::PlaySide::RIVAL:
    case RulesetBMS::PlaySide::NETWORK:
        assert(false);
        break;
    }
}

static std::optional<std::pair<RulesetBMS::JudgeArea, unsigned>> command_to_judge(ReplayChart::Commands::Type cmd)
{
    using CmdType = ReplayChart::Commands::Type;
    using JudgeArea = RulesetBMS::JudgeArea;
    switch (cmd)
    {
    case CmdType::JUDGE_LEFT_EXACT_0: return {{JudgeArea::EXACT_PERFECT, PLAYER_SLOT_PLAYER}};
    case CmdType::JUDGE_LEFT_EARLY_0: return {{JudgeArea::EARLY_PERFECT, PLAYER_SLOT_PLAYER}};
    case CmdType::JUDGE_LEFT_EARLY_1: return {{JudgeArea::EARLY_GREAT, PLAYER_SLOT_PLAYER}};
    case CmdType::JUDGE_LEFT_EARLY_2: return {{JudgeArea::EARLY_GOOD, PLAYER_SLOT_PLAYER}};
    case CmdType::JUDGE_LEFT_EARLY_3: return {{JudgeArea::EARLY_BAD, PLAYER_SLOT_PLAYER}};
    case CmdType::JUDGE_LEFT_EARLY_4: return {{JudgeArea::MISS, PLAYER_SLOT_PLAYER}};
    case CmdType::JUDGE_LEFT_EARLY_5: return {{JudgeArea::EARLY_KPOOR, PLAYER_SLOT_PLAYER}};
    case CmdType::JUDGE_LEFT_LATE_0: return {{JudgeArea::LATE_PERFECT, PLAYER_SLOT_PLAYER}};
    case CmdType::JUDGE_LEFT_LATE_1: return {{JudgeArea::LATE_GREAT, PLAYER_SLOT_PLAYER}};
    case CmdType::JUDGE_LEFT_LATE_2: return {{JudgeArea::LATE_GOOD, PLAYER_SLOT_PLAYER}};
    case CmdType::JUDGE_LEFT_LATE_3: return {{JudgeArea::LATE_BAD, PLAYER_SLOT_PLAYER}};
    case CmdType::JUDGE_LEFT_LATE_4: return {{JudgeArea::MISS, PLAYER_SLOT_PLAYER}};
    case CmdType::JUDGE_LEFT_LATE_5: return {{JudgeArea::EARLY_KPOOR, PLAYER_SLOT_PLAYER}};
    case CmdType::JUDGE_RIGHT_EXACT_0: return {{JudgeArea::EXACT_PERFECT, PLAYER_SLOT_TARGET}};
    case CmdType::JUDGE_RIGHT_EARLY_0: return {{JudgeArea::EARLY_PERFECT, PLAYER_SLOT_TARGET}};
    case CmdType::JUDGE_RIGHT_EARLY_1: return {{JudgeArea::EARLY_GREAT, PLAYER_SLOT_TARGET}};
    case CmdType::JUDGE_RIGHT_EARLY_2: return {{JudgeArea::EARLY_GOOD, PLAYER_SLOT_TARGET}};
    case CmdType::JUDGE_RIGHT_EARLY_3: return {{JudgeArea::EARLY_BAD, PLAYER_SLOT_TARGET}};
    case CmdType::JUDGE_RIGHT_EARLY_4: return {{JudgeArea::MISS, PLAYER_SLOT_TARGET}};
    case CmdType::JUDGE_RIGHT_EARLY_5: return {{JudgeArea::EARLY_KPOOR, PLAYER_SLOT_TARGET}};
    case CmdType::JUDGE_RIGHT_LATE_0: return {{JudgeArea::LATE_PERFECT, PLAYER_SLOT_TARGET}};
    case CmdType::JUDGE_RIGHT_LATE_1: return {{JudgeArea::LATE_GREAT, PLAYER_SLOT_TARGET}};
    case CmdType::JUDGE_RIGHT_LATE_2: return {{JudgeArea::LATE_GOOD, PLAYER_SLOT_TARGET}};
    case CmdType::JUDGE_RIGHT_LATE_3: return {{JudgeArea::LATE_BAD, PLAYER_SLOT_TARGET}};
    case CmdType::JUDGE_RIGHT_LATE_4: return {{JudgeArea::MISS, PLAYER_SLOT_TARGET}};
    case CmdType::JUDGE_RIGHT_LATE_5: return {{JudgeArea::EARLY_KPOOR, PLAYER_SLOT_TARGET}};
    case CmdType::JUDGE_LEFT_LANDMINE: return {{JudgeArea::MINE_KPOOR, PLAYER_SLOT_PLAYER}};
    case CmdType::JUDGE_RIGHT_LANDMINE: return {{JudgeArea::MINE_KPOOR, PLAYER_SLOT_TARGET}};
    default: return {};
    }
}

static ReplayChart::Commands::Type leftSideCmdToRight(const ReplayChart::Commands::Type cmd)
{
    using CmdType = ReplayChart::Commands::Type;
    switch (cmd)
    {
    case CmdType::S1L_DOWN: return CmdType::S2L_DOWN;
    case CmdType::S1R_DOWN: return CmdType::S2R_DOWN;
    case CmdType::K11_DOWN: return CmdType::K21_DOWN;
    case CmdType::K12_DOWN: return CmdType::K22_DOWN;
    case CmdType::K13_DOWN: return CmdType::K23_DOWN;
    case CmdType::K14_DOWN: return CmdType::K24_DOWN;
    case CmdType::K15_DOWN: return CmdType::K25_DOWN;
    case CmdType::K16_DOWN: return CmdType::K26_DOWN;
    case CmdType::K17_DOWN: return CmdType::K27_DOWN;
    case CmdType::K18_DOWN: return CmdType::K28_DOWN;
    case CmdType::K19_DOWN: return CmdType::K29_DOWN;
    case CmdType::S1L_UP: return CmdType::S2L_UP;
    case CmdType::S1R_UP: return CmdType::S2R_UP;
    case CmdType::K11_UP: return CmdType::K21_UP;
    case CmdType::K12_UP: return CmdType::K22_UP;
    case CmdType::K13_UP: return CmdType::K23_UP;
    case CmdType::K14_UP: return CmdType::K24_UP;
    case CmdType::K15_UP: return CmdType::K25_UP;
    case CmdType::K16_UP: return CmdType::K26_UP;
    case CmdType::K17_UP: return CmdType::K27_UP;
    case CmdType::K18_UP: return CmdType::K28_UP;
    case CmdType::K19_UP: return CmdType::K29_UP;
    case CmdType::S1A_PLUS: return CmdType::S2A_PLUS;
    case CmdType::S1A_MINUS: return CmdType::S2A_MINUS;
    case CmdType::S1A_STOP: return CmdType::S2A_STOP;
    case CmdType::JUDGE_LEFT_EXACT_0: return CmdType::JUDGE_RIGHT_EXACT_0;
    case CmdType::JUDGE_LEFT_EARLY_0: return CmdType::JUDGE_RIGHT_EARLY_0;
    case CmdType::JUDGE_LEFT_EARLY_1: return CmdType::JUDGE_RIGHT_EARLY_1;
    case CmdType::JUDGE_LEFT_EARLY_2: return CmdType::JUDGE_RIGHT_EARLY_2;
    case CmdType::JUDGE_LEFT_EARLY_3: return CmdType::JUDGE_RIGHT_EARLY_3;
    case CmdType::JUDGE_LEFT_EARLY_4: return CmdType::JUDGE_RIGHT_EARLY_4;
    case CmdType::JUDGE_LEFT_EARLY_5: return CmdType::JUDGE_RIGHT_EARLY_5;
    case CmdType::JUDGE_LEFT_LATE_0: return CmdType::JUDGE_RIGHT_LATE_0;
    case CmdType::JUDGE_LEFT_LATE_1: return CmdType::JUDGE_RIGHT_LATE_1;
    case CmdType::JUDGE_LEFT_LATE_2: return CmdType::JUDGE_RIGHT_LATE_2;
    case CmdType::JUDGE_LEFT_LATE_3: return CmdType::JUDGE_RIGHT_LATE_3;
    case CmdType::JUDGE_LEFT_LATE_4: return CmdType::JUDGE_RIGHT_LATE_4;
    case CmdType::JUDGE_LEFT_LATE_5: return CmdType::JUDGE_RIGHT_LATE_5;
    case CmdType::JUDGE_LEFT_LANDMINE: return CmdType::JUDGE_RIGHT_LANDMINE;
    default: return cmd;
    }
    abort(); // unreachable
}

void RulesetBMSReplay::update(const lunaticvibes::Time& t)
{
    const bool skipToEnd = t.hres() == LLONG_MAX;

    if (!_hasStartTime)
    {
        setStartTime(skipToEnd ? lunaticvibes::Time(0) : t);
    }

    auto rt = t - _startTime.norm();
    _basic.play_time = rt;
    using namespace chart;

    while (itReplayCommand != replay->commands.end() && rt.norm() >= (long long)std::round(itReplayCommand->ms * playbackSpeed / gSelectContext.pitchSpeed))
    {
        auto cmd = itReplayCommand->type;
        auto cmdTime = _startTime.norm() + itReplayCommand->ms;

        if (_side == PlaySide::AUTO_2P)
        {
            cmd = leftSideCmdToRight(cmd);
        }

        if (!skipToEnd)
        {
            if (gPlayContext.mode == SkinType::PLAY5 || gPlayContext.mode == SkinType::PLAY5_2)
            {
                if (auto it = REPLAY_CMD_INPUT_DOWN_MAP_5K[replayCmdMapIndex].find(cmd);
                    it != REPLAY_CMD_INPUT_DOWN_MAP_5K[replayCmdMapIndex].end())
                {
                    keyPressing[it->second] = true;
                }
                else if (REPLAY_CMD_INPUT_UP_MAP_5K[replayCmdMapIndex].find(cmd) != REPLAY_CMD_INPUT_UP_MAP_5K[replayCmdMapIndex].end())
                {
                    keyPressing[REPLAY_CMD_INPUT_UP_MAP_5K[replayCmdMapIndex].at(cmd)] = false;
                }
            }
            else
            {
                if (auto it = REPLAY_CMD_INPUT_DOWN_MAP.find(cmd); it != REPLAY_CMD_INPUT_DOWN_MAP.end())
                {
                    keyPressing[it->second] = true;
                }
                else if (REPLAY_CMD_INPUT_UP_MAP.find(cmd) != REPLAY_CMD_INPUT_UP_MAP.end())
                {
                    keyPressing[REPLAY_CMD_INPUT_UP_MAP.at(cmd)] = false;
                }
            }

            switch (cmd)
            {
            case ReplayChart::Commands::Type::S1A_PLUS:  playerScratchAccumulator[PLAYER_SLOT_PLAYER] = 0.0015; break;
            case ReplayChart::Commands::Type::S1A_MINUS: playerScratchAccumulator[PLAYER_SLOT_PLAYER] = -0.0015; break;
            case ReplayChart::Commands::Type::S1A_STOP:  playerScratchAccumulator[PLAYER_SLOT_PLAYER] = 0; break;
            case ReplayChart::Commands::Type::S2A_PLUS:  playerScratchAccumulator[PLAYER_SLOT_TARGET] = 0.0015; break;
            case ReplayChart::Commands::Type::S2A_MINUS: playerScratchAccumulator[PLAYER_SLOT_TARGET] = -0.0015; break;
            case ReplayChart::Commands::Type::S2A_STOP:  playerScratchAccumulator[PLAYER_SLOT_TARGET] = 0; break;

        default:
            break;
            }
        }

        if (auto judge = command_to_judge(cmd); judge.has_value())
            // TODO: pass lane index in here.
            updateJudge(cmdTime, NoteLaneIndex::_, judge->first, judge->second, true);

        itReplayCommand++;
    }

    InputMask pressed, released;
    if (!skipToEnd)
    {
        InputMask prev = keyPressing;
        pressed = keyPressing & ~prev;
        released = ~keyPressing & prev;
    }
    else
    {
        released = keyPressing;
    }
    if (pressed.any())
        RulesetBMS::updatePress(pressed, t);
    if (keyPressing.any())
        RulesetBMS::updateHold(keyPressing, t);
    if (released.any())
        RulesetBMS::updateRelease(released, t);

    RulesetBMS::update(t);
}

void RulesetBMSReplay::fail()
{
    _startTime = lunaticvibes::Time(0);
    _hasStartTime = true;
    RulesetBMS::fail();
}
