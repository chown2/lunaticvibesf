#include <gtest/gtest.h>

#include <common/types.h>
#include <game/lua_ir.h>
#include <game/ruleset/ruleset_bms.h>

#include <map>
#include <memory>

TEST(LuaIr, TestLuaIr)
{
    static const auto p = u8"../gamedata/resources/test_ir_clients/_test.lua"_p;

    auto client_opt = lunaticvibes::ir::Client::open(p);
    ASSERT_TRUE(client_opt.has_value());
    auto& client = *client_opt;

    EXPECT_EQ(client.id(), p);

    auto settings = client.get_settings();
    EXPECT_EQ(settings, (std::map<std::string, lunaticvibes::ir::SettingType>{
                            {"send_bogus", lunaticvibes::ir::SettingType::TEXT},
                            {"your_secret", lunaticvibes::ir::SettingType::PASSWORD}}));

    client.open_ir_page("feedfeedfeedfeedfeedfeedfeedfeed"); // zis shud rili tezt smthng

    {
        auto res = client.send_score(std::make_shared<ScoreBMS>(), std::make_shared<ChartFormatBMSMeta>(),
                                     lunaticvibes::BmsGaugeType::EASY, PlayModifierRandomType::RANDOM);
        EXPECT_EQ(res->place, 10);
        EXPECT_EQ(res->total_players, 200);
    }

    client.apply_settings({{"send_bogus", "true"}});
    {
        auto res = client.send_score(std::make_shared<ScoreBMS>(), std::make_shared<ChartFormatBMSMeta>(),
                                     lunaticvibes::BmsGaugeType::EASY, PlayModifierRandomType::RANDOM);
        EXPECT_EQ(res->place, 99999);
        EXPECT_EQ(res->total_players, 99999);
    }
}
