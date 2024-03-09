#include <gmock/gmock.h>

#include <common/utils.h>

TEST(Paths, ResolvesLr2PathsCorrectlty)
{
#ifdef _WIN32
    GTEST_SKIP() << "skipping the test on Windows as it expects Unix-style paths";
#endif // _WIN32

    // TODO: ban absolute paths.
    EXPECT_EQ(convertLR2Path("/home/me/lv", "/etc/passwd"), "/etc/passwd");
    EXPECT_EQ(convertLR2Path("/opt/lv", "/opt/lv/LR2files/Config/config.xml"), "/opt/lv/LR2files/Config/config.xml");

    EXPECT_EQ(convertLR2Path("/home/me/lv", ""), "");
    EXPECT_EQ(convertLR2Path("/home/me/lv", "."), ".");
    EXPECT_EQ(convertLR2Path("/home/me/lv", " "), " ");
    EXPECT_EQ(convertLR2Path("/home/me/lv", "  "), "  ");

    // FIXME: quotes only fail with global-buffer-overflow.
    // EXPECT_EQ(convertLR2Path("/home/me/lv", R"(" "");
    // EXPECT_EQ(convertLR2Path("/home/me/lv", R"("" ");
    // EXPECT_EQ(convertLR2Path("/home/me/lv", R"(""""""""""")"), "");
    EXPECT_EQ(convertLR2Path("/home/me/lv", R"(.\LR2files\Theme\EndlessCirculation\Play\parts\Combo\*.png""")"),
              "/home/me/lv/LR2files/Theme/EndlessCirculation/Play/parts/Combo/*.png");
}
