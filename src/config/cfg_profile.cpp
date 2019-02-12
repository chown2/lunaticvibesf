#include "cfg_profile.h"

ConfigProfile::ConfigProfile() : vConfig(CONFIG_FILE_PROFILE) {}
ConfigProfile::~ConfigProfile() {}

void ConfigProfile::setDefaults() noexcept
{
    using namespace cfg;
    _yaml[P_PLAYERNAME] = "Unnamed";
    _yaml[P_BASESPEED] = 1.0;
    _yaml[P_HISPEED] = 1.0;
    _yaml[P_REGULAR_SPEED] = 1200;
    _yaml[P_SPEED_TYPE] = P_SPEED_TYPE_NORMAL;
    _yaml[P_LANECOVER] = 0;
    _yaml[P_LIFT] = 0;
    _yaml[P_CHART_OP] = P_CHART_OP_NORMAL;
    _yaml[P_GAUGE_OP] = P_GAUGE_OP_NORMAL;
    _yaml[P_COMBO_POS_X] = 0;
    _yaml[P_COMBO_POS_Y] = 0;
    _yaml[P_JUDGE_POS_X] = 0;
    _yaml[P_JUDGE_POS_Y] = 0;
    _yaml[P_DISPLAY_LATENCY] = 0;
    _yaml[P_JUDGE_LATENCY] = 0;
    _yaml[P_JUDGE_STAT] = OFF;
    _yaml[P_GHOST_TYPE] = OFF;
    _yaml[P_GHOST_POS_X] = 0;
    _yaml[P_GHOST_POS_Y] = 0;
    _yaml[P_GHOST_TARGET] = 50;
    _yaml[P_GHOST_FS_TRIGGER] = OFF;
    _yaml[P_GHOST_FS_TYPE] = P_GHOST_FS_TYPE_NORMAL;
    _yaml[P_GHOST_FS_POS_X] = 0;
    _yaml[P_GHOST_FS_POS_Y] = 0;
}