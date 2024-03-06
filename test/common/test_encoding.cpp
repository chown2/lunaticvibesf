#include <fstream>

#include <boost/algorithm/string/trim.hpp>
#include <gmock/gmock.h>

#include <common/encoding.h>
#include <common/types.h>

TEST(Encoding, determine)
{
    GTEST_SKIP() << "Fiel encoding guessing is broken";
    EXPECT_EQ(getFileEncoding("encoding/euc_kr.txt"_p), eFileEncoding::EUC_KR);
    EXPECT_EQ(getFileEncoding("encoding/sjis.txt"_p), eFileEncoding::SHIFT_JIS);
    EXPECT_EQ(getFileEncoding("encoding/utf8.txt"_p), eFileEncoding::UTF8);
}

// Not about 'Encoding' per se but sure.
TEST(Encoding, unicode_path)
{
    Path file_path = std::filesystem::u8path("encoding/クールネーム.txt");
    std::ifstream ifs(file_path);
    ASSERT_FALSE(ifs.fail());
    std::string contents{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
    boost::trim(contents);

    EXPECT_STREQ(contents.c_str(), "it's so cool");
}
