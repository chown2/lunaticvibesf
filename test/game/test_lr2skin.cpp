#include "gmock/gmock.h"
#include "game/skin/skin_lr2.h"

#include <string>
#include <vector>

class mock_SkinLR2 : public SkinLR2
{
public:
	mock_SkinLR2(Path p, int loadMode = 0) : SkinLR2(p, loadMode) {}
	~mock_SkinLR2() override = default;


	std::vector<element> getDrawQueue() { return drawQueue; }
	const std::vector<std::string>& getHelpFiles() { return _helpFiles; }
};


TEST(tLR2Skin, IF1)
{
	std::shared_ptr<mock_SkinLR2> ps = nullptr;
	ASSERT_NO_THROW(ps = std::make_shared<mock_SkinLR2>("lr2skin/if1.lr2skin"));
	ASSERT_EQ(ps->isLoaded(), true);

	ASSERT_EQ(ps->getDrawQueue().size(), 1);
	const auto v = ps->getDrawQueue();
	EXPECT_EQ(v[0].op1, 1);
}

TEST(tLR2Skin, IF2)
{
	std::shared_ptr<mock_SkinLR2> ps = nullptr;
	ASSERT_NO_THROW(ps = std::make_shared<mock_SkinLR2>("lr2skin/if2.lr2skin"));
	ASSERT_EQ(ps->isLoaded(), true);

	ASSERT_EQ(ps->getDrawQueue().size(), 3);
	const auto v = ps->getDrawQueue();
	EXPECT_EQ(v[0].op1, 1);
	EXPECT_EQ(v[1].op1, 3);
	EXPECT_EQ(v[2].op1, 4);
}

TEST(tLR2Skin, IF3)
{
	std::shared_ptr<mock_SkinLR2> ps = nullptr;
	ASSERT_NO_THROW(ps = std::make_shared<mock_SkinLR2>("lr2skin/if3.lr2skin"));
	ASSERT_EQ(ps->isLoaded(), true);

	ASSERT_EQ(ps->getDrawQueue().size(), 2);
	const auto v = ps->getDrawQueue();
	EXPECT_EQ(v[0].op1, 3);
	EXPECT_EQ(v[1].op1, 5);
}

TEST(tLR2Skin, IF4)
{
	std::shared_ptr<mock_SkinLR2> ps = nullptr;
	ASSERT_NO_THROW(ps = std::make_shared<mock_SkinLR2>("lr2skin/if4.lr2skin"));
	ASSERT_EQ(ps->isLoaded(), true);

	ASSERT_EQ(ps->getDrawQueue().size(), 1);
	const auto v = ps->getDrawQueue();
	EXPECT_EQ(v[0].op1, 1);
}

TEST(tLR2Skin, IF5)
{
	GTEST_SKIP() << "FIXME: Broken test";
	std::shared_ptr<mock_SkinLR2> ps = nullptr;
	ASSERT_NO_THROW(ps = std::make_shared<mock_SkinLR2>("lr2skin/if5.lr2skin"));
	ASSERT_EQ(ps->isLoaded(), true);

	ASSERT_EQ(ps->getDrawQueue().size(), 3);
	const auto v = ps->getDrawQueue();
	EXPECT_EQ(v[1].op3, 0);
}

TEST(tLR2Skin, IF6)
{
	std::shared_ptr<mock_SkinLR2> ps = nullptr;
	ASSERT_NO_THROW(ps = std::make_shared<mock_SkinLR2>("lr2skin/if6.lr2skin"));
	ASSERT_EQ(ps->isLoaded(), true);

	ASSERT_EQ(ps->getDrawQueue().size(), 1);
	const auto v = ps->getDrawQueue();
	EXPECT_EQ(v[0].op1, 2);
}

TEST(tLR2Skin, HelpFileParsed)
{
	std::shared_ptr<mock_SkinLR2> ps = nullptr;
	ASSERT_NO_THROW(ps = std::make_shared<mock_SkinLR2>("lr2skin/helpfile.lr2skin"));
	ASSERT_EQ(ps->isLoaded(), true);

	static const std::vector<std::string> helpFiles{
		u8"「とわ」とわ\n",
		u8"(file error)",
		u8"「とわ」とわ\n",
		u8"(file error)",
		u8"(file error)",
		u8"(file error)",
		u8"(file error)",
		u8"(file error)",
		u8"(file error)",
		u8"UTF-8\nテスト\n",
		// No tenth.
	};

	EXPECT_EQ(ps->getHelpFiles(), helpFiles);
}
