#include <gmock/gmock.h>

#include <common/utils.h>

TEST(Paths, ResolvesLr2PathsCorrectlty)
{
    // TODO: ban absolute paths.
    EXPECT_EQ(convertLR2Path("/home/me/lv", "/etc/passwd"), "/etc/passwd");
    EXPECT_EQ(convertLR2Path("/opt/lv", "/opt/lv/LR2files/Config/config.xml"), "/opt/lv/LR2files/Config/config.xml");
#ifdef _WIN32
    EXPECT_EQ(convertLR2Path("D:\\Games\\LunaticVibes", "C:\\something\\somewhere"), "C:\\something\\somewhere");
    EXPECT_EQ(convertLR2Path("D:\\Games\\LunaticVibes", R"(D:\Games\LunaticVibes\LR2files\Config\config.xml)"),
              R"(D:\Games\LunaticVibes\LR2files\Config\config.xml)");
#else
    EXPECT_EQ(convertLR2Path("D:\\Games\\LunaticVibes", "C:\\something\\somewhere"), "C:/something/somewhere");
    EXPECT_EQ(convertLR2Path("D:\\Games\\LunaticVibes", R"(D:\Games\LunaticVibes\LR2files\Config\config.xml)"),
              R"(D:/Games/LunaticVibes/LR2files/Config/config.xml)");
#endif // _WIN32

    EXPECT_EQ(convertLR2Path("/home/me/lv", ""), "");
    EXPECT_EQ(convertLR2Path("/home/me/lv", "."), ".");
    EXPECT_EQ(convertLR2Path("/home/me/lv", " "), " ");
    EXPECT_EQ(convertLR2Path("/home/me/lv", "  "), "  ");
    EXPECT_EQ(convertLR2Path("D:\\Games\\LunaticVibes", ""), "");
    EXPECT_EQ(convertLR2Path("D:\\Games\\LunaticVibes", "."), ".");
    EXPECT_EQ(convertLR2Path("D:\\Games\\LunaticVibes", " "), " ");
    EXPECT_EQ(convertLR2Path("D:\\Games\\LunaticVibes", "  "), "  ");

    // FIXME: quotes only fail with global-buffer-overflow.
    // EXPECT_EQ(convertLR2Path("/home/me/lv", R"(" "");
    // EXPECT_EQ(convertLR2Path("/home/me/lv", R"("" ");
    // EXPECT_EQ(convertLR2Path("/home/me/lv", R"(""""""""""")"), "");
#ifdef _WIN32
    EXPECT_EQ(
        convertLR2Path("D:\\Games\\LunaticVibes", R"(.\LR2files\Theme\EndlessCirculation\Play\parts\Combo\*.png""")"),
        R"(D:\Games\LunaticVibes\LR2files\Theme\EndlessCirculation\Play\parts\Combo\*.png)");
#else
    EXPECT_EQ(convertLR2Path("/home/me/lv", R"(.\LR2files\Theme\EndlessCirculation\Play\parts\Combo\*.png""")"),
              "/home/me/lv/LR2files/Theme/EndlessCirculation/Play/parts/Combo/*.png");
#endif // _WIN32
}
