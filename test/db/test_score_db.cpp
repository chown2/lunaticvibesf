#include <gmock/gmock.h>

#include <db/db_score.h>

static constexpr auto&& IN_MEMORY_DB_PATH = ":memory:";

// TODO: make these tests use some real-life data.

TEST(ScoreDb, ChartScoreUpdating)
{
    static const HashMD5 hash = md5("deadbeef");

    ScoreDB score_db { IN_MEMORY_DB_PATH };
    ScoreBMS score;
    score.exscore = 1;

    EXPECT_EQ(score_db.getChartScoreBMS(hash), nullptr);

    score_db.updateChartScoreBMS(hash, score);
    EXPECT_EQ(score_db.getChartScoreBMS(hash)->exscore, 1);

    score.exscore = 2;
    score_db.updateChartScoreBMS(hash, score);
    EXPECT_EQ(score_db.getChartScoreBMS(hash)->exscore, 2);

    score.exscore = 1;
    score_db.updateChartScoreBMS(hash, score);
    EXPECT_EQ(score_db.getChartScoreBMS(hash)->exscore, 2);
}

TEST(ScoreDb, CourseScoreUpdating)
{
    static const HashMD5 hash = md5("deadbeef");

    ScoreDB score_db { IN_MEMORY_DB_PATH };
    ScoreBMS score;
    score.exscore = 1;

    EXPECT_EQ(score_db.getCourseScoreBMS(hash), nullptr);

    score_db.updateCourseScoreBMS(hash, score);
    EXPECT_EQ(score_db.getCourseScoreBMS(hash)->exscore, 1);

    score.exscore = 2;
    score_db.updateCourseScoreBMS(hash, score);
    EXPECT_EQ(score_db.getCourseScoreBMS(hash)->exscore, 2);

    score.exscore = 1;
    score_db.updateCourseScoreBMS(hash, score);
    EXPECT_EQ(score_db.getCourseScoreBMS(hash)->exscore, 2);
}

TEST(ScoreDb, ChartScoreDeleting)
{
    static const HashMD5 hash = md5("deadbeef");

    ScoreDB score_db { IN_MEMORY_DB_PATH };
    ScoreBMS score;
    score.exscore = 1;

    EXPECT_EQ(score_db.getChartScoreBMS(hash), nullptr);

    score_db.updateChartScoreBMS(hash, score);
    EXPECT_EQ(score_db.getChartScoreBMS(hash)->exscore, 1);

    score_db.deleteChartScoreBMS(hash);
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
    EXPECT_EQ(score_db.getCourseScoreBMS(hash)->exscore, 1);

    score_db.deleteCourseScoreBMS(hash);
    EXPECT_EQ(score_db.getCourseScoreBMS(hash), nullptr);
}
