#include <gmock/gmock.h>

#include <db/db_score.h>

static constexpr auto&& IN_MEMORY_DB_PATH = ":memory:";

// TODO: make these tests use some real-life data.

TEST(ScoreDb, ChartScoreUpdating)
{
    static const HashMD5 hash = md5("deadbeef");

    ScoreDB score_db { IN_MEMORY_DB_PATH };

    EXPECT_EQ(score_db.getChartScoreBMS(hash), nullptr);
    EXPECT_EQ(score_db.getStats().pgreat, 0);

    {
        ScoreBMS score;
        score.exscore = 1;
        score.lamp = ScoreBMS::Lamp::NOPLAY;
        score.pgreat = 2;
        score.replayFileName = "score1";
        score_db.insertChartScoreBMS(hash, score);
        EXPECT_EQ(score_db.getChartScoreBMS(hash)->exscore, 1);
        EXPECT_EQ(score_db.getStats().pgreat, 0);
        EXPECT_EQ(score_db.getStats().play_count, 0);
        EXPECT_EQ(score_db.getStats().clear_count, 0);
    }

    {
        ScoreBMS score;
        score.exscore = 2;
        score.lamp = ScoreBMS::Lamp::FAILED;
        score.pgreat = 2;
        score.replayFileName = "score2";
        score_db.insertChartScoreBMS(hash, score);
        EXPECT_EQ(score_db.getChartScoreBMS(hash)->exscore, 2);
        EXPECT_EQ(score_db.getStats().pgreat, 2);
        EXPECT_EQ(score_db.getStats().play_count, 1);
        EXPECT_EQ(score_db.getStats().clear_count, 0);
    }

    {
        ScoreBMS score;
        score.exscore = 1;
        score.lamp = ScoreBMS::Lamp::EASY;
        score.pgreat = 2;
        score.replayFileName = "score3";
        score_db.insertChartScoreBMS(hash, score);
        EXPECT_EQ(score_db.getChartScoreBMS(hash)->exscore, 2);
        EXPECT_EQ(score_db.getStats().pgreat, 4);
        EXPECT_EQ(score_db.getStats().play_count, 2);
        EXPECT_EQ(score_db.getStats().clear_count, 1);
    }

    // Cache reloading works.
    score_db.preloadScore();
    ASSERT_NE(score_db.getChartScoreBMS(hash), nullptr);
    EXPECT_EQ(score_db.getChartScoreBMS(hash)->exscore, 2);
}

TEST(ScoreDb, CourseScoreUpdating)
{
    static const HashMD5 hash = md5("deadbeef");

    ScoreDB score_db { IN_MEMORY_DB_PATH };
    ScoreBMS score;
    score.exscore = 1;
    score.lamp = ScoreBMS::Lamp::EASY;
    score.pgreat = 2;

    EXPECT_EQ(score_db.getCourseScoreBMS(hash), nullptr);
    EXPECT_EQ(score_db.getStats().pgreat, 0);

    score_db.updateCourseScoreBMS(hash, score);
    EXPECT_EQ(score_db.getCourseScoreBMS(hash)->exscore, 1);
    EXPECT_EQ(score_db.getStats().pgreat, 0);

    score.exscore = 2;
    score_db.updateCourseScoreBMS(hash, score);
    EXPECT_EQ(score_db.getCourseScoreBMS(hash)->exscore, 2);
    EXPECT_EQ(score_db.getStats().pgreat, 0);

    score.exscore = 1;
    score_db.updateCourseScoreBMS(hash, score);
    EXPECT_EQ(score_db.getCourseScoreBMS(hash)->exscore, 2);
    EXPECT_EQ(score_db.getStats().pgreat, 0);

    // Cache reloading works.
    score_db.preloadScore();
    ASSERT_NE(score_db.getCourseScoreBMS(hash), nullptr);
    EXPECT_EQ(score_db.getCourseScoreBMS(hash)->exscore, 2);
}

TEST(ScoreDb, ChartScoreDeleting)
{
    static const HashMD5 hash = md5("deadbeef");

    ScoreDB score_db { IN_MEMORY_DB_PATH };
    ScoreBMS score;
    score.exscore = 1;

    EXPECT_EQ(score_db.getChartScoreBMS(hash), nullptr);

    // And also test for old DB table. Same scores themselves are never supposed to be in both.
    score_db.updateLegacyChartScoreBMS(hash, score);
    score_db.insertChartScoreBMS(hash, score);
    ASSERT_NE(score_db.getChartScoreBMS(hash), nullptr);
    EXPECT_EQ(score_db.getChartScoreBMS(hash)->exscore, 1);

    score_db.deleteChartScoreBMS(hash);
    EXPECT_EQ(score_db.getChartScoreBMS(hash), nullptr);

    // Not just cache that was invalidated.
    score_db.rebuildBmsPbCache();
    EXPECT_EQ(score_db.getChartScoreBMS(hash), nullptr);
}

TEST(ScoreDb, CourseScoreDeleting)
{
    static const HashMD5 hash = md5("deadbeef");

    ScoreDB score_db { IN_MEMORY_DB_PATH };
    ScoreBMS score;
    score.exscore = 1;

    EXPECT_EQ(score_db.getCourseScoreBMS(hash), nullptr);

    score_db.updateCourseScoreBMS(hash, score);
    EXPECT_NE(score_db.getCourseScoreBMS(hash), nullptr);
    EXPECT_EQ(score_db.getCourseScoreBMS(hash)->exscore, 1);

    score_db.deleteCourseScoreBMS(hash);
    EXPECT_EQ(score_db.getCourseScoreBMS(hash), nullptr);

    // Not just cache that was invalidated.
    score_db.rebuildBmsPbCache();
    EXPECT_EQ(score_db.getChartScoreBMS(hash), nullptr);
}
