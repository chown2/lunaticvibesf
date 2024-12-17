#include "cfg_ir.h"

#include <common/meta.h>
#include <common/types.h>
#include <common/utils.h>

static StringPath make_cfg_file(const StringPathView& profile, const std::string& ir_id)
{
    fs::create_directories(Path(GAMEDATA_PATH) / "profile" / profile / "ir");
    Path p;
    p /= "ir/";
    p /= PathFromUTF8(ir_id).filename().replace_extension(".yml");
    return p.native();
}

lunaticvibes::ConfigIr::ConfigIr(const std::string& profile, const std::string& ir_id)
    : vConfig(profile, make_cfg_file(profile, ir_id))
{
    load();
}
lunaticvibes::ConfigIr::~ConfigIr()
{
    save();
}

void lunaticvibes::ConfigIr::setDefaults() noexcept
{
    set("enabled", false);
}
