#include <gmock/gmock.h>

#include <optional>

#include <common/entry/entry_course.h>
#include <common/entry/entry_song.h>
#include <common/hash.h>
#include <game/scene/scene_context.h>
#include <game/scene/scene_select.h>

static constexpr auto &&IN_MEMORY_DB_PATH = ":memory:";

TEST(SceneSelect, SearchQueryScoreDeletionWorks)
{
    using lunaticvibes::DeleteScoreResult;

    SelectContextParams select_context;
    SongDB song_db{IN_MEMORY_DB_PATH};
    ScoreDB score_db{IN_MEMORY_DB_PATH};

    auto chart = std::make_shared<ChartFormatBase>();
    chart->fileHash = md5("not actually a file");

    CourseLr2crs::Course course;
    course.hashTop = md5("this isn't actually supposed to be a hash lmao").hexdigest();

    select_context.entries.emplace_back(std::make_shared<EntryChart>(chart), nullptr);
    select_context.entries.emplace_back(std::make_shared<EntryCourse>(course, 0), nullptr);
    select_context.entries.emplace_back(std::make_shared<EntryBase>(), nullptr);

    ScoreBMS chart_score;
    chart_score.exscore = 1;
    EXPECT_EQ(score_db.getChartScoreBMS(chart->fileHash), nullptr);
    score_db.updateChartScoreBMS(chart->fileHash, chart_score);
    EXPECT_EQ(score_db.getChartScoreBMS(chart->fileHash)->exscore, 1);

    ScoreBMS course_score;
    course_score.exscore = 2;
    EXPECT_EQ(score_db.getCourseScoreBMS(course.getCourseHash()), nullptr);
    score_db.updateCourseScoreBMS(course.getCourseHash(), course_score);
    EXPECT_EQ(score_db.getCourseScoreBMS(course.getCourseHash())->exscore, 2);

    {
        select_context.selectedEntryIndex = 0;
        auto result_variant = lunaticvibes::execute_search_query(select_context, song_db, score_db, "/deletescore");
        DeleteScoreResult result;
        EXPECT_NO_THROW(result = std::get<DeleteScoreResult>(result_variant));
        EXPECT_EQ(result, DeleteScoreResult::Ok);
        EXPECT_EQ(score_db.getChartScoreBMS(chart->fileHash), nullptr);
    }

    {
        select_context.selectedEntryIndex = 1;
        auto result_variant = lunaticvibes::execute_search_query(select_context, song_db, score_db, "/deletescore");
        DeleteScoreResult result;
        EXPECT_NO_THROW(result = std::get<DeleteScoreResult>(result_variant));
        EXPECT_EQ(result, DeleteScoreResult::Ok);
        EXPECT_EQ(score_db.getCourseScoreBMS(course.getCourseHash()), nullptr);
    }

    {
        select_context.selectedEntryIndex = 2;
        auto result_variant = lunaticvibes::execute_search_query(select_context, song_db, score_db, "/deletescore");
        DeleteScoreResult result;
        EXPECT_NO_THROW(result = std::get<DeleteScoreResult>(result_variant));
        EXPECT_EQ(result, DeleteScoreResult::Error);
    }
}

TEST(SceneSelect, HashQueryingWorks)
{
    using lunaticvibes::HashResult;

    SelectContextParams select_context;
    SongDB song_db{IN_MEMORY_DB_PATH};
    ScoreDB score_db{IN_MEMORY_DB_PATH};

    auto chart = std::make_shared<ChartFormatBase>();
    chart->fileHash = md5("not actually a file");

    CourseLr2crs::Course course;
    course.hashTop = md5("this isn't actually supposed to be a hash lmao").hexdigest();

    select_context.entries.emplace_back(std::make_shared<EntryChart>(chart), nullptr);
    select_context.entries.emplace_back(std::make_shared<EntryCourse>(course, 0), nullptr);
    select_context.entries.emplace_back(std::make_shared<EntryBase>(), nullptr);

    {
        select_context.selectedEntryIndex = 0;
        auto result_variant = lunaticvibes::execute_search_query(select_context, song_db, score_db, "/hash");
        HashResult result;
        EXPECT_NO_THROW(result = std::get<HashResult>(result_variant));
        EXPECT_EQ(result.hash, chart->fileHash.hexdigest());
    }

    {
        select_context.selectedEntryIndex = 1;
        auto result_variant = lunaticvibes::execute_search_query(select_context, song_db, score_db, "/hash");
        HashResult result;
        EXPECT_NO_THROW(result = std::get<HashResult>(result_variant));
        // TODO: probably show something for courses too.
        EXPECT_EQ(result.hash, std::nullopt);
    }

    {
        select_context.selectedEntryIndex = 2;
        auto result_variant = lunaticvibes::execute_search_query(select_context, song_db, score_db, "/hash");
        HashResult result;
        EXPECT_NO_THROW(result = std::get<HashResult>(result_variant));
        EXPECT_EQ(result.hash, std::nullopt);
    }
}
