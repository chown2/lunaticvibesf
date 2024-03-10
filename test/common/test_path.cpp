#include <gmock/gmock.h>

#include <common/types.h>
#include <common/utils.h>

TEST(Paths, ResolvesLr2PathsCorrectlty)
{
// fs::path::is_absolute only works for native paths, tested on libstdc++ with GCC 13.2.1 and some MSVC version.
#ifdef _WIN32
    EXPECT_EQ(convertLR2Path("D:\\Games\\LunaticVibes", "C:\\something\\somewhere"), "");
    EXPECT_EQ(convertLR2Path("D:\\Games\\LunaticVibes", Path { "C:\\something\\somewhere" }), "");
    EXPECT_EQ(convertLR2Path("D:\\Games\\LunaticVibes", R"(D:\Games\LunaticVibes\LR2files\Config\config.xml)"), "");
#else
    EXPECT_EQ(convertLR2Path("/home/me/lv", "/etc/passwd"), "");
    EXPECT_EQ(convertLR2Path("/home/me/lv", Path { "/etc/passwd" }), "");
    EXPECT_EQ(convertLR2Path("/opt/lv", "/opt/lv/LR2files/Config/config.xml"), "");
#endif // _WIN32

    EXPECT_EQ(convertLR2Path("/home/me/lv", u8" テスト "), u8" テスト ");

    EXPECT_EQ(convertLR2Path("/home/me/lv", ""), "");
    EXPECT_EQ(convertLR2Path("/home/me/lv", "."), ".");
    EXPECT_EQ(convertLR2Path("/home/me/lv", " "), " ");
    EXPECT_EQ(convertLR2Path("/home/me/lv", "  "), "  ");
    EXPECT_EQ(convertLR2Path("D:\\Games\\LunaticVibes", ""), "");
    EXPECT_EQ(convertLR2Path("D:\\Games\\LunaticVibes", "."), ".");
    EXPECT_EQ(convertLR2Path("D:\\Games\\LunaticVibes", " "), " ");
    EXPECT_EQ(convertLR2Path("D:\\Games\\LunaticVibes", "  "), "  ");

    EXPECT_EQ(convertLR2Path("/home/me/lv", "\" "), " ");
    EXPECT_EQ(convertLR2Path("/home/me/lv", " \""), " ");
    EXPECT_EQ(convertLR2Path("/home/me/lv", R"(""""""""""")"), "");
#ifdef _WIN32
    EXPECT_EQ(
        convertLR2Path("D:\\Games\\LunaticVibes", R"(.\LR2files\Theme\EndlessCirculation\Play\parts\Combo\*.png""")"),
        R"(D:\Games\LunaticVibes\LR2files\Theme\EndlessCirculation\Play\parts\Combo\*.png)");
#else
    EXPECT_EQ(convertLR2Path("/home/me/lv", R"(.\LR2files\Theme\EndlessCirculation\Play\parts\Combo\*.png""")"),
        "/home/me/lv/LR2files/Theme/EndlessCirculation/Play/parts/Combo/*.png");
#endif // _WIN32
}
