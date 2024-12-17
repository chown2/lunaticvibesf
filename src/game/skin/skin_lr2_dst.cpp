#include <common/assert.h>
#include <game/arena/arena_data.h>
#include <game/lua_ir_global.h>
#include <game/ruleset/ruleset_bms.h>
#include <game/scene/scene_context.h>
#include <game/skin/skin_lr2.h>

namespace
{
static std::shared_mutex _mutex;
static std::bitset<900> _op;
static std::bitset<900> _opIsCached;
static std::bitset<100> _customOp;
} // namespace

[[nodiscard]] static bool any_of(std::initializer_list<unsigned> entries, unsigned val)
{
    for (auto e : entries)
        if (e == val)
            return true;
    return false;
}

static int ceilingForChartPlayKeys()
{
    switch (State::get(IndexOption::CHART_PLAY_KEYS))
    {
        using namespace Option;
    case KEYS_7:
    case KEYS_14: return 12; break;
    case KEYS_5:
    case KEYS_10: return 9; break;
    case KEYS_9: return 42; break;
    case KEYS_24:
    case KEYS_48: return 90; break;
    }
    return 12;
}

static bool getDstOptAbs(unsigned d)
{
    using namespace Option;

    // 1000: arena Online
    // FIXME
    // if (gArenaData.isOnline())
    // {
    //     for (size_t i = 0; i < MAX_ARENA_PLAYERS; ++i)
    //     {
    //         if (auto pr = gArenaData.getPlayerRuleset(i); pr)
    //         {
    //             const auto d = pr->getData();
    //             int offset = 50 * i;
    //
    //             using namespace Option;
    //             switch (Option::getRankType(d.acc))
    //             {
    //             case RANK_NONE: break;
    //             case RANK_0:
    //             case RANK_1: set(1011 + offset); break;
    //             case RANK_2: set(1012 + offset); break;
    //             case RANK_3: set(1013 + offset); break;
    //             case RANK_4: set(1014 + offset); break;
    //             case RANK_5: set(1015 + offset); break;
    //             case RANK_6: set(1016 + offset); break;
    //             case RANK_7: set(1017 + offset); break;
    //             case RANK_8: set(1018 + offset); break;
    //             }
    //             switch (Option::getRankType(d.total_acc))
    //             {
    //             case RANK_NONE: break;
    //             case RANK_0:
    //             case RANK_1: set(1021 + offset); break;
    //             case RANK_2: set(1022 + offset); break;
    //             case RANK_3: set(1023 + offset); break;
    //             case RANK_4: set(1024 + offset); break;
    //             case RANK_5: set(1025 + offset); break;
    //             case RANK_6: set(1026 + offset); break;
    //             case RANK_7: set(1027 + offset); break;
    //             case RANK_8: set(1028 + offset); break;
    //             }
    //             switch (Option::getHealthType(d.health))
    //             {
    //             case HEALTH_0: set(1030 + offset); break;
    //             case HEALTH_10: set(1031 + offset); break;
    //             case HEALTH_20: set(1032 + offset); break;
    //             case HEALTH_30: set(1033 + offset); break;
    //             case HEALTH_40: set(1034 + offset); break;
    //             case HEALTH_50: set(1035 + offset); break;
    //             case HEALTH_60: set(1036 + offset); break;
    //             case HEALTH_70: set(1037 + offset); break;
    //             case HEALTH_80: set(1038 + offset); break;
    //             case HEALTH_90: set(1039 + offset); break;
    //             case HEALTH_100: set(1040 + offset); break;
    //             }
    //
    //             if (d.judge[RulesetBMS::JUDGE_CB] == 0)
    //             {
    //                 set(1045 + offset);
    //             }
    //             else if (d.health >= pr->getClearHealth())
    //             {
    //                 if (auto prb = std::dynamic_pointer_cast<RulesetBMS>(gArenaData.getPlayerRuleset(i)); prb)
    //                 {
    //                     switch (prb->getGaugeType())
    //                     {
    //                     case RulesetBMS::GaugeType::GROOVE: set(1043 + offset); break;
    //                     case RulesetBMS::GaugeType::EASY: set(1042 + offset); break;
    //                     case RulesetBMS::GaugeType::ASSIST: set(1047 + offset); break;
    //                     case RulesetBMS::GaugeType::HARD: set(1044 + offset); break;
    //                     case RulesetBMS::GaugeType::EXHARD: set(1046 + offset); break;
    //
    //                     case RulesetBMS::GaugeType::DEATH:
    //                     case RulesetBMS::GaugeType::P_ATK:
    //                     case RulesetBMS::GaugeType::G_ATK:
    //                     case RulesetBMS::GaugeType::GRADE:
    //                     case RulesetBMS::GaugeType::EXGRADE: break;
    //                     }
    //                 }
    //             }
    //             else
    //             {
    //                 set(1041 + offset);
    //             }
    //         }
    //
    //         set(1401 + i, gArenaData.isPlayerReady(i));
    //     }
    // }

    switch (d)
    {
    case 0: return true;
    case 1: return State::get(IndexOption::SELECT_ENTRY_TYPE) == ENTRY_FOLDER;
    case 2: return State::get(IndexOption::SELECT_ENTRY_TYPE) == ENTRY_SONG;
    case 3: return State::get(IndexOption::SELECT_ENTRY_TYPE) == ENTRY_COURSE;
    case 4: return State::get(IndexOption::SELECT_ENTRY_TYPE) == ENTRY_NEW_COURSE;
    case 5: return any_of({ENTRY_SONG, ENTRY_COURSE}, State::get(IndexOption::SELECT_ENTRY_TYPE));
    case 10:
        return any_of({PLAY_MODE_DOUBLE, PLAY_MODE_DOUBLE_BATTLE, PLAY_MODE_DP_GHOST_BATTLE},
                      State::get(IndexOption::PLAY_MODE));
    case 11: return State::get(IndexOption::PLAY_MODE) == PLAY_MODE_BATTLE;
    case 12:
        return any_of({PLAY_MODE_BATTLE, PLAY_MODE_DOUBLE_BATTLE, PLAY_MODE_DP_GHOST_BATTLE},
                      State::get(IndexOption::PLAY_MODE));
    case 13:
        return any_of({PLAY_MODE_BATTLE, PLAY_MODE_SP_GHOST_BATTLE, PLAY_MODE_DP_GHOST_BATTLE},
                      State::get(IndexOption::PLAY_MODE));
    case 20:
        return State::get(IndexSwitch::SELECT_PANEL1) || State::get(IndexSwitch::SELECT_PANEL2) ||
               State::get(IndexSwitch::SELECT_PANEL3) || State::get(IndexSwitch::SELECT_PANEL4) ||
               State::get(IndexSwitch::SELECT_PANEL5) || State::get(IndexSwitch::SELECT_PANEL6) ||
               State::get(IndexSwitch::SELECT_PANEL7) || State::get(IndexSwitch::SELECT_PANEL8) ||
               State::get(IndexSwitch::SELECT_PANEL9);
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29: return State::get((IndexSwitch)(d - 21 + (unsigned)IndexSwitch::SELECT_PANEL1));
    case 30: return State::get(IndexOption::PLAY_BGA_SIZE) == BGA_NORMAL;
    case 31: return State::get(IndexOption::PLAY_BGA_SIZE) == BGA_EXTEND;
    case 32: return !State::get(IndexSwitch::SYSTEM_AUTOPLAY);
    case 33: return State::get(IndexSwitch::SYSTEM_AUTOPLAY);
    case 34: return State::get(IndexOption::PLAY_GHOST_TYPE_1P) == GHOST_OFF;
    case 35: return State::get(IndexOption::PLAY_GHOST_TYPE_1P) == GHOST_TOP;
    case 36: return State::get(IndexOption::PLAY_GHOST_TYPE_1P) == GHOST_SIDE;
    case 37: return State::get(IndexOption::PLAY_GHOST_TYPE_1P) == GHOST_SIDE_BOTTOM;
    case 38: return !State::get(IndexSwitch::SYSTEM_SCOREGRAPH);
    case 39: return State::get(IndexSwitch::SYSTEM_SCOREGRAPH);
    case 40: return !State::get(IndexSwitch::_LOAD_BGA);
    case 41: return State::get(IndexSwitch::_LOAD_BGA);
    case 42: return any_of({GAUGE_ASSISTEASY, GAUGE_EASY, GAUGE_NORMAL}, State::get(IndexOption::PLAY_GAUGE_TYPE_1P));
    case 43: return any_of({GAUGE_HARD, GAUGE_EXHARD, GAUGE_DEATH}, State::get(IndexOption::PLAY_GAUGE_TYPE_1P));
    case 44: return any_of({GAUGE_ASSISTEASY, GAUGE_EASY, GAUGE_NORMAL}, State::get(IndexOption::PLAY_GAUGE_TYPE_2P));
    case 45: return any_of({GAUGE_HARD, GAUGE_EXHARD, GAUGE_DEATH}, State::get(IndexOption::PLAY_GAUGE_TYPE_2P));
    case 46: return true; // 46 難易度フィルタが有効 // NOTE: It's an option of LR2 setup, not a runtime stat. Fix to ON
    case 47: return false; // 47 難易度フィルタが無効
    case 48: return any_of({GAUGE_EXHARD, GAUGE_DEATH}, State::get(IndexOption::PLAY_GAUGE_TYPE_1P)); // lunaticvibes
    case 49: return any_of({GAUGE_EXHARD, GAUGE_DEATH}, State::get(IndexOption::PLAY_GAUGE_TYPE_2P)); // lunaticvibes
    case 50: return !getDstOptAbs(51);
    case 51: return lunaticvibes::get_global_ir_context()->is_online();
    case 52: return !State::get(IndexSwitch::PLAY_OPTION_EXTRA);
    case 53: return State::get(IndexSwitch::PLAY_OPTION_EXTRA);
    case 54: return !State::get(IndexSwitch::PLAY_OPTION_AUTOSCR_1P);
    case 55: return State::get(IndexSwitch::PLAY_OPTION_AUTOSCR_1P);
    case 56: return !State::get(IndexSwitch::PLAY_OPTION_AUTOSCR_2P);
    case 57: return State::get(IndexSwitch::PLAY_OPTION_AUTOSCR_2P);
    case 60: return !State::get(IndexSwitch::CHART_CAN_SAVE_SCORE);
    case 61: return State::get(IndexSwitch::CHART_CAN_SAVE_SCORE);
    case 62: return any_of({LAMP_NOPLAY, LAMP_FAILED}, State::get(IndexOption::CHART_SAVE_LAMP_TYPE));
    case 63: return any_of({LAMP_ASSIST, LAMP_EASY}, State::get(IndexOption::CHART_SAVE_LAMP_TYPE));
    case 64: return any_of({LAMP_NORMAL}, State::get(IndexOption::CHART_SAVE_LAMP_TYPE));
    case 65: return any_of({LAMP_HARD, LAMP_EXHARD}, State::get(IndexOption::CHART_SAVE_LAMP_TYPE));
    case 66: return any_of({LAMP_FULLCOMBO, LAMP_PERFECT, LAMP_MAX}, State::get(IndexOption::CHART_SAVE_LAMP_TYPE));
    case 70: return State::get(IndexNumber::MUSIC_BEGINNER_LEVEL) <= ceilingForChartPlayKeys();
    case 71: return State::get(IndexNumber::MUSIC_NORMAL_LEVEL) <= ceilingForChartPlayKeys();
    case 72: return State::get(IndexNumber::MUSIC_HYPER_LEVEL) <= ceilingForChartPlayKeys();
    case 73: return State::get(IndexNumber::MUSIC_ANOTHER_LEVEL) <= ceilingForChartPlayKeys();
    case 74: return State::get(IndexNumber::MUSIC_INSANE_LEVEL) <= ceilingForChartPlayKeys();
    case 75: return State::get(IndexNumber::MUSIC_BEGINNER_LEVEL) > ceilingForChartPlayKeys();
    case 76: return State::get(IndexNumber::MUSIC_NORMAL_LEVEL) > ceilingForChartPlayKeys();
    case 77: return State::get(IndexNumber::MUSIC_HYPER_LEVEL) > ceilingForChartPlayKeys();
    case 78: return State::get(IndexNumber::MUSIC_ANOTHER_LEVEL) > ceilingForChartPlayKeys();
    case 79: return State::get(IndexNumber::MUSIC_INSANE_LEVEL) > ceilingForChartPlayKeys();
    case 80: return any_of({SPLAY_PREPARE, SPLAY_LOADING}, State::get(IndexOption::PLAY_SCENE_STAT));
    case 81: return !any_of({SPLAY_PREPARE, SPLAY_LOADING}, State::get(IndexOption::PLAY_SCENE_STAT));
    case 82: return false; // リプレイオフ
    case 83: return !State::get(IndexSwitch::SYSTEM_AUTOPLAY) && !gPlayContext.isReplay;
    case 84: return !State::get(IndexSwitch::SYSTEM_AUTOPLAY) && gPlayContext.isReplay;
    case 90: return State::get(IndexSwitch::RESULT_CLEAR);
    case 91: return !State::get(IndexSwitch::RESULT_CLEAR);
    case 100: return getDstOptAbs(5) && State::get(IndexOption::SELECT_ENTRY_LAMP) == LAMP_NOPLAY;
    case 101: return State::get(IndexOption::SELECT_ENTRY_LAMP) == LAMP_FAILED;
    case 102: return any_of({LAMP_EASY, LAMP_ASSIST}, State::get(IndexOption::SELECT_ENTRY_LAMP));
    case 103: return State::get(IndexOption::SELECT_ENTRY_LAMP) == LAMP_NORMAL;
    case 104: return any_of({LAMP_EXHARD, LAMP_HARD}, State::get(IndexOption::SELECT_ENTRY_LAMP));
    case 105: return any_of({LAMP_FULLCOMBO, LAMP_MAX, LAMP_PERFECT}, State::get(IndexOption::SELECT_ENTRY_LAMP));
    case 106: return State::get(IndexOption::SELECT_ENTRY_LAMP) == LAMP_EXHARD;  // lunaticvibes
    case 107: return State::get(IndexOption::SELECT_ENTRY_LAMP) == LAMP_PERFECT; // lunaticvibes
    case 108: return State::get(IndexOption::SELECT_ENTRY_LAMP) == LAMP_MAX;     // lunaticvibes
    case 109: return State::get(IndexOption::SELECT_ENTRY_LAMP) == LAMP_ASSIST;  // lunaticvibes
    case 110: return State::get(IndexOption::SELECT_ENTRY_RANK) == RANK_1;
    case 111: return State::get(IndexOption::SELECT_ENTRY_RANK) == RANK_2;
    case 112: return State::get(IndexOption::SELECT_ENTRY_RANK) == RANK_3;
    case 113: return State::get(IndexOption::SELECT_ENTRY_RANK) == RANK_4;
    case 114: return State::get(IndexOption::SELECT_ENTRY_RANK) == RANK_5;
    case 115: return State::get(IndexOption::SELECT_ENTRY_RANK) == RANK_6;
    case 116: return State::get(IndexOption::SELECT_ENTRY_RANK) == RANK_7;
    case 117: return State::get(IndexOption::SELECT_ENTRY_RANK) == RANK_8;
    // 118 GROOVE //クリア済みオプションフラグ(ゲージ)
    // 119 SURVIVAL
    // 120 SUDDEN DEATH
    // 121 EASY
    // 122 PERFECT ATTACK
    // 123 GOOD ATTACK
    // 126 正規 //クリア済みオプションフラグ(ランダム)
    // 127 MIRROR
    // 128 RANDOM
    // 129 S-RANDOM
    // 130 SCATTER
    // 131 CONVERGE
    // 134 無し //クリア済みオプションフラグ(エフェクト)
    // 135 HIDDEN
    // 136 SUDDEN
    // 137 HID+SUD
    // 142 AUTO SCRATCH (自動皿抜きでクリアすれば消えます) //その他オプションフラグ
    // 143 EXTRA MODE
    // 144 DOUBLE BATTLE
    // 145 SP TO DP (もしかしたら今後DP TO SPや 9 TO 7と共有項目になるかも。
    case 146: return State::get(IndexOption::SELECT_ENTRY_LAMP) == LAMP_EASY;      // lunaticvibes
    case 147: return State::get(IndexOption::SELECT_ENTRY_LAMP) == LAMP_HARD;      // lunaticvibes
    case 148: return State::get(IndexOption::SELECT_ENTRY_LAMP) == LAMP_FULLCOMBO; // lunaticvibes
    case 150: return getDstOptAbs(5) && State::get(IndexOption::CHART_DIFFICULTY) == DIFF_ANY;
    case 151: return getDstOptAbs(5) && State::get(IndexOption::CHART_DIFFICULTY) == DIFF_BEGINNER;
    case 152: return getDstOptAbs(5) && State::get(IndexOption::CHART_DIFFICULTY) == DIFF_NORMAL;
    case 153: return getDstOptAbs(5) && State::get(IndexOption::CHART_DIFFICULTY) == DIFF_HYPER;
    case 154: return getDstOptAbs(5) && State::get(IndexOption::CHART_DIFFICULTY) == DIFF_ANOTHER;
    case 155: return getDstOptAbs(5) && State::get(IndexOption::CHART_DIFFICULTY) == DIFF_INSANE;
    case 160: return getDstOptAbs(5) && State::get(IndexOption::CHART_PLAY_KEYS) == KEYS_7;
    case 161: return getDstOptAbs(5) && State::get(IndexOption::CHART_PLAY_KEYS) == KEYS_5;
    case 162: return getDstOptAbs(5) && State::get(IndexOption::CHART_PLAY_KEYS) == KEYS_14;
    case 163: return getDstOptAbs(5) && State::get(IndexOption::CHART_PLAY_KEYS) == KEYS_10;
    case 164: return getDstOptAbs(5) && State::get(IndexOption::CHART_PLAY_KEYS) == KEYS_9;
    case 165:               // オプション全適用後の最終的な鍵盤数 165 7keys
    case 166:               // 166 5keys
    case 167:               // 167 14keys
    case 168:               // 168 10keys
    case 169: return false; // 169 9keys
    case 170: return !State::get(IndexSwitch::CHART_HAVE_BGA);
    case 171: return State::get(IndexSwitch::CHART_HAVE_BGA);
    case 172: return !State::get(IndexSwitch::CHART_HAVE_LN);
    case 173: return State::get(IndexSwitch::CHART_HAVE_LN);
    case 174: return !State::get(IndexSwitch::CHART_HAVE_README);
    case 175: return State::get(IndexSwitch::CHART_HAVE_README);
    case 176: return !State::get(IndexSwitch::CHART_HAVE_BPMCHANGE);
    case 177: return State::get(IndexSwitch::CHART_HAVE_BPMCHANGE);
    case 178: return !State::get(IndexSwitch::CHART_HAVE_RANDOM);
    case 179: return State::get(IndexSwitch::CHART_HAVE_RANDOM);
    case 180: return getDstOptAbs(5) && State::get(IndexOption::CHART_JUDGE_TYPE) == JUDGE_VHARD;
    case 181: return getDstOptAbs(5) && State::get(IndexOption::CHART_JUDGE_TYPE) == JUDGE_HARD;
    case 182: return getDstOptAbs(5) && State::get(IndexOption::CHART_JUDGE_TYPE) == JUDGE_NORMAL;
    case 183: return getDstOptAbs(5) && State::get(IndexOption::CHART_JUDGE_TYPE) == JUDGE_EASY;
    case 184: return false; // VERYEASY (beatoraja extension)
    case 185:
        switch (State::get(IndexOption::CHART_DIFFICULTY))
        {
        case DIFF_BEGINNER: return getDstOptAbs(70);
        case DIFF_NORMAL: return getDstOptAbs(71);
        case DIFF_HYPER: return getDstOptAbs(72);
        case DIFF_ANOTHER: return getDstOptAbs(73);
        case DIFF_INSANE: return getDstOptAbs(74);
        }
        lunaticvibes::verify_failed("185?");
        return false;
    case 186:
        switch (State::get(IndexOption::CHART_DIFFICULTY))
        {
        case DIFF_BEGINNER: return getDstOptAbs(75);
        case DIFF_NORMAL: return getDstOptAbs(76);
        case DIFF_HYPER: return getDstOptAbs(77);
        case DIFF_ANOTHER: return getDstOptAbs(78);
        case DIFF_INSANE: return getDstOptAbs(79);
        }
        lunaticvibes::verify_failed("186?");
        return false;
    case 190: return !State::get(IndexSwitch::CHART_HAVE_STAGEFILE);
    case 191: return State::get(IndexSwitch::CHART_HAVE_STAGEFILE);
    case 192: return !State::get(IndexSwitch::CHART_HAVE_BANNER);
    case 193: return State::get(IndexSwitch::CHART_HAVE_BANNER);
    case 194: return !State::get(IndexSwitch::CHART_HAVE_BACKBMP);
    case 195: return State::get(IndexSwitch::CHART_HAVE_BACKBMP);
    case 196: return !State::get(IndexSwitch::CHART_HAVE_REPLAY);
    case 197: return State::get(IndexSwitch::CHART_HAVE_REPLAY);
    case 200: return State::get(IndexOption::PLAY_RANK_ESTIMATED_1P) == RANK_1;
    case 201: return State::get(IndexOption::PLAY_RANK_ESTIMATED_1P) == RANK_2;
    case 202: return State::get(IndexOption::PLAY_RANK_ESTIMATED_1P) == RANK_3;
    case 203: return State::get(IndexOption::PLAY_RANK_ESTIMATED_1P) == RANK_4;
    case 204: return State::get(IndexOption::PLAY_RANK_ESTIMATED_1P) == RANK_5;
    case 205: return State::get(IndexOption::PLAY_RANK_ESTIMATED_1P) == RANK_6;
    case 206: return State::get(IndexOption::PLAY_RANK_ESTIMATED_1P) == RANK_7;
    case 207: return State::get(IndexOption::PLAY_RANK_ESTIMATED_1P) == RANK_8;
    case 210: return State::get(IndexOption::PLAY_RANK_ESTIMATED_2P) == RANK_1;
    case 211: return State::get(IndexOption::PLAY_RANK_ESTIMATED_2P) == RANK_2;
    case 212: return State::get(IndexOption::PLAY_RANK_ESTIMATED_2P) == RANK_3;
    case 213: return State::get(IndexOption::PLAY_RANK_ESTIMATED_2P) == RANK_4;
    case 214: return State::get(IndexOption::PLAY_RANK_ESTIMATED_2P) == RANK_5;
    case 215: return State::get(IndexOption::PLAY_RANK_ESTIMATED_2P) == RANK_6;
    case 216: return State::get(IndexOption::PLAY_RANK_ESTIMATED_2P) == RANK_7;
    case 217: return State::get(IndexOption::PLAY_RANK_ESTIMATED_2P) == RANK_8;
    case 220:                                                                     // AAA
    case 221:                                                                     // AA
    case 222:                                                                     // A
    case 223:                                                                     // B
    case 224:                                                                     // C
    case 225:                                                                     // D
    case 226:                                                                     // E
    case 227: return State::get(IndexOption::PLAY_RANK_BORDER_1P) <= d + 1 - 220; // F // See enum e_rank_type.
    case 230: return State::get(IndexOption::PLAY_HEALTH_1P) == HEALTH_0;
    case 231: return State::get(IndexOption::PLAY_HEALTH_1P) == HEALTH_10;
    case 232: return State::get(IndexOption::PLAY_HEALTH_1P) == HEALTH_20;
    case 233: return State::get(IndexOption::PLAY_HEALTH_1P) == HEALTH_30;
    case 234: return State::get(IndexOption::PLAY_HEALTH_1P) == HEALTH_40;
    case 235: return State::get(IndexOption::PLAY_HEALTH_1P) == HEALTH_50;
    case 236: return State::get(IndexOption::PLAY_HEALTH_1P) == HEALTH_60;
    case 237: return State::get(IndexOption::PLAY_HEALTH_1P) == HEALTH_70;
    case 238: return State::get(IndexOption::PLAY_HEALTH_1P) == HEALTH_80;
    case 239: return State::get(IndexOption::PLAY_HEALTH_1P) == HEALTH_90;
    case 240: return State::get(IndexOption::PLAY_HEALTH_1P) == HEALTH_100;
    case 241: return State::get(IndexOption::PLAY_LAST_JUDGE_1P) == JUDGE_0;
    case 242: return State::get(IndexOption::PLAY_LAST_JUDGE_1P) == JUDGE_1;
    case 243: return State::get(IndexOption::PLAY_LAST_JUDGE_1P) == JUDGE_2;
    case 244: return State::get(IndexOption::PLAY_LAST_JUDGE_1P) == JUDGE_3;
    case 245: return State::get(IndexOption::PLAY_LAST_JUDGE_1P) == JUDGE_4;
    case 246: return State::get(IndexOption::PLAY_LAST_JUDGE_1P) == JUDGE_5;
    case 247: return true;  // 267 1P POORBGA表示時間外
    case 248: return false; // 268 1P POORBGA表示時間中
    case 250: return State::get(IndexOption::PLAY_HEALTH_2P) == HEALTH_0;
    case 251: return State::get(IndexOption::PLAY_HEALTH_2P) == HEALTH_10;
    case 252: return State::get(IndexOption::PLAY_HEALTH_2P) == HEALTH_20;
    case 253: return State::get(IndexOption::PLAY_HEALTH_2P) == HEALTH_30;
    case 254: return State::get(IndexOption::PLAY_HEALTH_2P) == HEALTH_40;
    case 255: return State::get(IndexOption::PLAY_HEALTH_2P) == HEALTH_50;
    case 256: return State::get(IndexOption::PLAY_HEALTH_2P) == HEALTH_60;
    case 257: return State::get(IndexOption::PLAY_HEALTH_2P) == HEALTH_70;
    case 258: return State::get(IndexOption::PLAY_HEALTH_2P) == HEALTH_80;
    case 259: return State::get(IndexOption::PLAY_HEALTH_2P) == HEALTH_90;
    case 260: return State::get(IndexOption::PLAY_HEALTH_2P) == HEALTH_100;
    case 261: return State::get(IndexOption::PLAY_LAST_JUDGE_2P) == JUDGE_0;
    case 262: return State::get(IndexOption::PLAY_LAST_JUDGE_2P) == JUDGE_1;
    case 263: return State::get(IndexOption::PLAY_LAST_JUDGE_2P) == JUDGE_2;
    case 264: return State::get(IndexOption::PLAY_LAST_JUDGE_2P) == JUDGE_3;
    case 265: return State::get(IndexOption::PLAY_LAST_JUDGE_2P) == JUDGE_4;
    case 266: return State::get(IndexOption::PLAY_LAST_JUDGE_2P) == JUDGE_5;
    case 267: return true;  // 267 2P POORBGA表示時間外
    case 268: return false; // 268 2P POORBGA表示時間中
    case 270: return State::get(IndexSwitch::P1_SETTING_LANECOVER);
    case 271: return State::get(IndexSwitch::P2_SETTING_LANECOVER);
    case 272: return State::get(IndexSwitch::P1_SETTING_HISPEED);
    case 273: return State::get(IndexSwitch::P2_SETTING_HISPEED);
    case 274: return State::get(IndexSwitch::P1_HAS_LANECOVER_TOP);
    case 275: return State::get(IndexSwitch::P2_HAS_LANECOVER_TOP);
    case 276: return State::get(IndexSwitch::P1_HAS_LANECOVER_BOTTOM);
    case 277: return State::get(IndexSwitch::P2_HAS_LANECOVER_BOTTOM);
    case 280: return State::get(IndexOption::PLAY_COURSE_STAGE) == STAGE_1;
    case 281: return State::get(IndexOption::PLAY_COURSE_STAGE) == STAGE_2;
    case 282: return State::get(IndexOption::PLAY_COURSE_STAGE) == STAGE_3;
    case 283: return State::get(IndexOption::PLAY_COURSE_STAGE) == STAGE_4;
    case 289: // NOTE: LR2 handles single song as FINAL
        return any_of({STAGE_FINAL, STAGE_NOT_COURSE}, State::get(IndexOption::PLAY_COURSE_STAGE));
    case 290: return State::get(IndexOption::SELECT_ENTRY_TYPE) == Option::ENTRY_COURSE;
    case 291:
        return State::get(IndexOption::SELECT_ENTRY_TYPE) == Option::ENTRY_COURSE &&
               State::get(IndexOption::COURSE_TYPE) == Option::COURSE_NONSTOP;
    case 292:
        return State::get(IndexOption::SELECT_ENTRY_TYPE) == Option::ENTRY_COURSE &&
               State::get(IndexOption::COURSE_TYPE) == Option::COURSE_EXPERT;
    case 293:
        return State::get(IndexOption::SELECT_ENTRY_TYPE) == Option::ENTRY_COURSE &&
               State::get(IndexOption::COURSE_TYPE) == Option::COURSE_GRADE;
    case 300: return any_of({RANK_1, RANK_0}, State::get(IndexOption::RESULT_RANK_1P));
    case 301: return State::get(IndexOption::RESULT_RANK_1P) == RANK_2;
    case 302: return State::get(IndexOption::RESULT_RANK_1P) == RANK_3;
    case 303: return State::get(IndexOption::RESULT_RANK_1P) == RANK_4;
    case 304: return State::get(IndexOption::RESULT_RANK_1P) == RANK_5;
    case 305: return State::get(IndexOption::RESULT_RANK_1P) == RANK_6;
    case 306: return State::get(IndexOption::RESULT_RANK_1P) == RANK_7;
    case 307: return State::get(IndexOption::RESULT_RANK_1P) == RANK_8;
    case 308: return State::get(IndexOption::RESULT_RANK_1P) == RANK_NONE;
    case 310: return any_of({RANK_1, RANK_0}, State::get(IndexOption::RESULT_RANK_2P));
    case 311: return State::get(IndexOption::RESULT_RANK_2P) == RANK_2;
    case 312: return State::get(IndexOption::RESULT_RANK_2P) == RANK_3;
    case 313: return State::get(IndexOption::RESULT_RANK_2P) == RANK_4;
    case 314: return State::get(IndexOption::RESULT_RANK_2P) == RANK_5;
    case 315: return State::get(IndexOption::RESULT_RANK_2P) == RANK_6;
    case 316: return State::get(IndexOption::RESULT_RANK_2P) == RANK_7;
    case 317: return State::get(IndexOption::RESULT_RANK_2P) == RANK_8;
    case 318: return State::get(IndexOption::RESULT_RANK_2P) == RANK_NONE;
    case 320: return any_of({RANK_1, RANK_0}, State::get(IndexOption::RESULT_MYBEST_RANK));
    case 321: return State::get(IndexOption::RESULT_MYBEST_RANK) == RANK_2;
    case 322: return State::get(IndexOption::RESULT_MYBEST_RANK) == RANK_3;
    case 323: return State::get(IndexOption::RESULT_MYBEST_RANK) == RANK_4;
    case 324: return State::get(IndexOption::RESULT_MYBEST_RANK) == RANK_5;
    case 325: return State::get(IndexOption::RESULT_MYBEST_RANK) == RANK_6;
    case 326: return State::get(IndexOption::RESULT_MYBEST_RANK) == RANK_7;
    case 327: return any_of({RANK_8, RANK_NONE}, State::get(IndexOption::RESULT_MYBEST_RANK));
    case 330: return State::get(IndexSwitch::RESULT_UPDATED_SCORE);
    case 331: return State::get(IndexSwitch::RESULT_UPDATED_MAXCOMBO);
    case 332: return State::get(IndexSwitch::RESULT_UPDATED_BP);
    case 333: return State::get(IndexSwitch::RESULT_UPDATED_TRIAL);
    case 334: return State::get(IndexSwitch::RESULT_UPDATED_IRRANK);
    case 335: return State::get(IndexSwitch::RESULT_UPDATED_RANK);
    case 340: return any_of({RANK_1, RANK_0}, State::get(IndexOption::RESULT_UPDATED_RANK));
    case 341: return State::get(IndexOption::RESULT_UPDATED_RANK) == RANK_2;
    case 342: return State::get(IndexOption::RESULT_UPDATED_RANK) == RANK_3;
    case 343: return State::get(IndexOption::RESULT_UPDATED_RANK) == RANK_4;
    case 344: return State::get(IndexOption::RESULT_UPDATED_RANK) == RANK_5;
    case 345: return State::get(IndexOption::RESULT_UPDATED_RANK) == RANK_6;
    case 346: return State::get(IndexOption::RESULT_UPDATED_RANK) == RANK_7;
    case 347: return State::get(IndexOption::RESULT_UPDATED_RANK) == RANK_8;
    case 350: return State::get(IndexSwitch::FLIP_RESULT);
    case 351: return !State::get(IndexSwitch::FLIP_RESULT);
    case 352: return State::get(IndexOption::RESULT_BATTLE_WIN_LOSE) == 1;
    case 353: return State::get(IndexOption::RESULT_BATTLE_WIN_LOSE) == 2;
    case 354: return State::get(IndexOption::RESULT_BATTLE_WIN_LOSE) == 0;
    case 400: return State::get(IndexOption::KEY_CONFIG_MODE) == KEYCFG_7;
    case 401: return State::get(IndexOption::KEY_CONFIG_MODE) == KEYCFG_9;
    case 402: return State::get(IndexOption::KEY_CONFIG_MODE) == KEYCFG_5;
    case 500: return getDstOptAbs(2) && !State::get(IndexSwitch::CHART_HAVE_DIFFICULTY_1);
    case 501: return getDstOptAbs(2) && !State::get(IndexSwitch::CHART_HAVE_DIFFICULTY_2);
    case 502: return getDstOptAbs(2) && !State::get(IndexSwitch::CHART_HAVE_DIFFICULTY_3);
    case 503: return getDstOptAbs(2) && !State::get(IndexSwitch::CHART_HAVE_DIFFICULTY_4);
    case 504: return getDstOptAbs(2) && !State::get(IndexSwitch::CHART_HAVE_DIFFICULTY_5);
    case 505: return getDstOptAbs(2) && State::get(IndexSwitch::CHART_HAVE_DIFFICULTY_1);
    case 506: return getDstOptAbs(2) && State::get(IndexSwitch::CHART_HAVE_DIFFICULTY_2);
    case 507: return getDstOptAbs(2) && State::get(IndexSwitch::CHART_HAVE_DIFFICULTY_3);
    case 508: return getDstOptAbs(2) && State::get(IndexSwitch::CHART_HAVE_DIFFICULTY_4);
    case 509: return getDstOptAbs(2) && State::get(IndexSwitch::CHART_HAVE_DIFFICULTY_5);
    case 510: return getDstOptAbs(505) && !State::get(IndexSwitch::CHART_HAVE_MULTIPLE_DIFFICULTY_1);
    case 511: return getDstOptAbs(506) && !State::get(IndexSwitch::CHART_HAVE_MULTIPLE_DIFFICULTY_2);
    case 512: return getDstOptAbs(507) && !State::get(IndexSwitch::CHART_HAVE_MULTIPLE_DIFFICULTY_3);
    case 513: return getDstOptAbs(508) && !State::get(IndexSwitch::CHART_HAVE_MULTIPLE_DIFFICULTY_4);
    case 514: return getDstOptAbs(509) && !State::get(IndexSwitch::CHART_HAVE_MULTIPLE_DIFFICULTY_5);
    case 515: return getDstOptAbs(505) && State::get(IndexSwitch::CHART_HAVE_MULTIPLE_DIFFICULTY_1);
    case 516: return getDstOptAbs(506) && State::get(IndexSwitch::CHART_HAVE_MULTIPLE_DIFFICULTY_2);
    case 517: return getDstOptAbs(507) && State::get(IndexSwitch::CHART_HAVE_MULTIPLE_DIFFICULTY_3);
    case 518: return getDstOptAbs(508) && State::get(IndexSwitch::CHART_HAVE_MULTIPLE_DIFFICULTY_4);
    case 519: return getDstOptAbs(509) && State::get(IndexSwitch::CHART_HAVE_MULTIPLE_DIFFICULTY_5);
    // 520 レベルバー beginner no play
    // 521 レベルバー beginner failed
    // 522 レベルバー beginner easy
    // 523 レベルバー beginner clear
    // 524 レベルバー beginner hardclear
    // 525 レベルバー beginner fullcombo
    // 530 レベルバー normal no play
    // 531 レベルバー normal failed
    // 532 レベルバー normal easy
    // 533 レベルバー normal clear
    // 534 レベルバー normal hardclear
    // 535 レベルバー normal fullcombo
    // 540 レベルバー hyper no play
    // 541 レベルバー hyper failed
    // 542 レベルバー hyper easy
    // 543 レベルバー hyper clear
    // 544 レベルバー hyper hardclear
    // 545 レベルバー hyper fullcombo
    // 550 レベルバー another no play
    // 551 レベルバー another failed
    // 552 レベルバー another easy
    // 553 レベルバー another clear
    // 554 レベルバー another hardclear
    // 555 レベルバー another fullcombo
    // 560 レベルバー insane no play
    // 561 レベルバー insane failed
    // 562 レベルバー insane easy
    // 563 レベルバー insane clear
    // 564 レベルバー insane hardclear
    // 565 レベルバー insane fullcombo
    case 571: return false; // コースセレクト中である
    case 572: return true;  // コースセレクト中では無い
    case 580:
    case 581:
    case 582:
    case 583:
    case 584:
    case 585:
    case 586:
    case 587:
    case 588:
    case 589: return State::get(IndexOption::COURSE_STAGE_COUNT) - 580;
    // 590 コースセレクト stage1選択中
    // 591 コースセレクト stage2選択中
    // 592 コースセレクト stage3選択中
    // 593 コースセレクト stage4選択中
    // 594 コースセレクト stage5選択中
    // 595 コースセレクト stage6選択中
    // 596 コースセレクト stage7選択中
    // 597 コースセレクト stage8選択中
    // 598 コースセレクト stage9選択中
    // 599 コースセレクト stage10選択中
    case 600: return true;  // 600 IR対象ではない
    case 601:               // 601 IR読み込み中
    case 602:               // 602 IR読み込み完了
    case 603:               // 603 IRプレイヤー無し
    case 604:               // 604 IR接続失敗
    case 605:               // 605 BAN曲
    case 606:               // 606 IR更新待ち
    case 607:               // 607 IRアクセス中
    case 608: return false; // 608 IRビジー
    case 620: return true;  // ランキング表示中ではない
    case 621: return false; // ランキング表示中
    case 622: return true;  // ゴーストバトルではない
    case 623: return false; // ゴーストバトル発動中(決定演出～リザルトの間のみ)
    case 624: return true; // 自分と相手のスコアを比較する状況ではない (現状では、ランキング表示中とライバルフォルダ)
    case 625:
        return false; // 自分と相手のスコアを比較するべき状況である

        // 640 NOT PLAYED rival
        // 641 FAILED
        // 642 EASY CLEARED
        // 643 NORMAL CLEARED
        // 644 HARD CLEARED
        // 645 FULL COMBO
        // 650 AAA 8/9 rival
        // 651 AA 7/9
        // 652 A 6/9
        // 653 B 5/9
        // 654 C 4/9
        // 655 D 3/9
        // 656 E 2/9
        // 657 F 1/9

        // LR2HelperG DST_OPTION HS-FIX 720-724
        // case 720: return State::get(IndexOption::PLAY_HSFIX_TYPE_1P) == Option::SPEED_NORMAL;
        // case 721: return State::get(IndexOption::PLAY_HSFIX_TYPE_1P) == Option::SPEED_FIX_MIN;
        // case 722: return State::get(IndexOption::PLAY_HSFIX_TYPE_1P) == Option::SPEED_FIX_MAX;
        // case 723: return State::get(IndexOption::PLAY_HSFIX_TYPE_1P) == Option::SPEED_FIX_AVG;
        // case 724: return State::get(IndexOption::PLAY_HSFIX_TYPE_1P) == Option::SPEED_FIX_CONSTANT;

    case 700:
        return State::get(IndexSwitch::COURSE_STAGE1_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE1_DIFFICULTY) == 0;
    case 701:
        return State::get(IndexSwitch::COURSE_STAGE1_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE1_DIFFICULTY) == 1;
    case 702:
        return State::get(IndexSwitch::COURSE_STAGE1_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE1_DIFFICULTY) == 2;
    case 703:
        return State::get(IndexSwitch::COURSE_STAGE1_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE1_DIFFICULTY) == 3;
    case 704:
        return State::get(IndexSwitch::COURSE_STAGE1_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE1_DIFFICULTY) == 4;
    case 705:
        return State::get(IndexSwitch::COURSE_STAGE1_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE1_DIFFICULTY) == 5;
    case 710:
        return State::get(IndexSwitch::COURSE_STAGE2_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE2_DIFFICULTY) == 0;
    case 711:
        return State::get(IndexSwitch::COURSE_STAGE2_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE2_DIFFICULTY) == 1;
    case 712:
        return State::get(IndexSwitch::COURSE_STAGE2_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE2_DIFFICULTY) == 2;
    case 713:
        return State::get(IndexSwitch::COURSE_STAGE2_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE2_DIFFICULTY) == 3;
    case 714:
        return State::get(IndexSwitch::COURSE_STAGE2_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE2_DIFFICULTY) == 4;
    case 715:
        return State::get(IndexSwitch::COURSE_STAGE2_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE2_DIFFICULTY) == 5;
    case 720:
        return State::get(IndexSwitch::COURSE_STAGE3_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE3_DIFFICULTY) == 0;
    case 721:
        return State::get(IndexSwitch::COURSE_STAGE3_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE3_DIFFICULTY) == 1;
    case 722:
        return State::get(IndexSwitch::COURSE_STAGE3_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE3_DIFFICULTY) == 2;
    case 723:
        return State::get(IndexSwitch::COURSE_STAGE3_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE3_DIFFICULTY) == 3;
    case 724:
        return State::get(IndexSwitch::COURSE_STAGE3_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE3_DIFFICULTY) == 4;
    case 725:
        return State::get(IndexSwitch::COURSE_STAGE3_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE3_DIFFICULTY) == 5;
    case 730:
        return State::get(IndexSwitch::COURSE_STAGE4_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE4_DIFFICULTY) == 0;
    case 731:
        return State::get(IndexSwitch::COURSE_STAGE4_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE4_DIFFICULTY) == 1;
    case 732:
        return State::get(IndexSwitch::COURSE_STAGE4_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE4_DIFFICULTY) == 2;
    case 733:
        return State::get(IndexSwitch::COURSE_STAGE4_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE4_DIFFICULTY) == 3;
    case 734:
        return State::get(IndexSwitch::COURSE_STAGE4_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE4_DIFFICULTY) == 4;
    case 735:
        return State::get(IndexSwitch::COURSE_STAGE4_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE4_DIFFICULTY) == 5;
    case 740:
        return State::get(IndexSwitch::COURSE_STAGE5_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE5_DIFFICULTY) == 0;
    case 741:
        return State::get(IndexSwitch::COURSE_STAGE5_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE5_DIFFICULTY) == 1;
    case 742:
        return State::get(IndexSwitch::COURSE_STAGE5_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE5_DIFFICULTY) == 2;
    case 743:
        return State::get(IndexSwitch::COURSE_STAGE5_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE5_DIFFICULTY) == 3;
    case 744:
        return State::get(IndexSwitch::COURSE_STAGE5_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE5_DIFFICULTY) == 4;
    case 745:
        return State::get(IndexSwitch::COURSE_STAGE5_CHART_EXIST) &&
               State::get(IndexOption::COURSE_STAGE5_DIFFICULTY) == 5;
    case 800: return State::get(IndexSwitch::P1_LANECOVER_ENABLED); // lunaticvibes
    case 801: return State::get(IndexSwitch::P1_LOCK_SPEED);        // lunaticvibes
    case 810: return State::get(IndexSwitch::P2_LANECOVER_ENABLED); // lunaticvibes
    case 811: return State::get(IndexSwitch::P2_LOCK_SPEED);        // lunaticvibes
    case 999: return false;
    case 1000: return gArenaData.isOnline(); // lunaticvibes
    case 1001:
    case 1002:
    case 1003:
    case 1004:
    case 1005:
    case 1006:
    case 1007:
    case 1008: return gArenaData.isOnline() && gArenaData.getPlayerCount() > d - 1000; // lunaticvibes
    case 1400: return gArenaData.isOnline() && gArenaData.isSelfReady();
    case 9999: return true; // Is lunaticvibes.
    default:
        if (d >= 900 && d <= 999)
        {
            std::shared_lock l(_mutex);
            return _customOp[d - 900];
        }
        return false;
    }
}

bool getDstOpt(int d)
{
    auto op = (dst_option)std::abs(d);

    if (std::shared_lock l{_mutex}; d < static_cast<int>(_opIsCached.size()) && _opIsCached[op])
    {
        l.unlock();
        const bool result = _op[op];
        return (d >= 0) ? result : !result;
    }

    const bool result = getDstOptAbs(op);
    std::unique_lock l{_mutex};
    if (op < static_cast<int>(_op.size()))
    {
        _op[op] = result;
        _opIsCached[op] = true;
    }
    l.unlock();
    return (d >= 0) ? result : !result;
}

void setCustomDstOpt(unsigned base, size_t offset, bool val)
{
    if (base + offset < 900 || base + offset > 999)
        return;
    std::unique_lock l(_mutex);
    _customOp[base + offset - 900] = val;
}

void clearCustomDstOpt()
{
    std::unique_lock l(_mutex);
    _customOp.reset();
}

void updateDstOpt()
{
    std::unique_lock l(_mutex);
    _opIsCached.reset();
}
