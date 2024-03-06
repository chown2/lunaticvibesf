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
