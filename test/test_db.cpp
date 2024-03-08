#include "gmock/gmock.h"
#include "db/db_song.h"

static constexpr auto&& pathSongDB = ":memory:";

TEST(SongDB, table_song)
{
    //std::filesystem::current_path("F:\\banshao_test");
    SongDB db{ pathSongDB };

    // TODO

    //ASSERT_TRUE(std::filesystem::exists("res/test"));

    //auto hash = db.getFolderHash("res/test/db_song");
    //auto list = db.browse(hash);
    //ASSERT_FALSE(list.empty());

    //ASSERT_EQ(4, list.getContentsCount());
}

TEST(SongDB, remove_table)
{
    //std::filesystem::current_path("F:\\banshao_test");
    SongDB db{ pathSongDB };

    // TODO 
    // 
    //auto hash = db.getFolderHash("res/test");
    //ASSERT_FALSE(hash.empty());
    //ASSERT_EQ(0, db.removeFolder(hash, true));

    //auto hash1 = db.getFolderHash(std::filesystem::absolute("res/test").u8string());
    //ASSERT_FALSE(hash1.empty());
    //ASSERT_EQ(0, db.removeFolder(hash1, true));
}
