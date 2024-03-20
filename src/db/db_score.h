#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <stdint.h>

#include "common/types.h"
#include "common/hash.h"
#include "db/db_conn.h"

class ScoreBase;
class ScoreBMS;

namespace lunaticvibes {

struct OverallStats
{
    int64_t play_count;
    int64_t clear_count;
    int64_t pgreat;
    int64_t great;
    int64_t good;
    int64_t bad;
    int64_t poor;
    int64_t running_combo; // Accumulates between plays.
    int64_t max_running_combo;
    int64_t playtime; // In milliseconds (LR2 used seconds).
};

} // namespace lunaticvibes

/* TABLE classic_chart:
    md5(TEXT), totalnotes(INTEGER), score(INTEGER), rate(REAL), reserved[1-4](INTEGER), reserved[5-6](REAL)
    exscore(INTEGER), lamp(INTEGER), pgreat(INTEGER), great(INTEGER), good(INTEGER), bad(INTEGER), kpoor(INTEGER), miss(INTEGER)
    md5: hash string, calculated by "relative path to exe" OR "absolute path"
    ...
*/
class ScoreDB : public SQLite
{
protected:
    mutable std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<ScoreBMS>>> cache;

public:
    ScoreDB() = delete;
    ScoreDB(const char* path);
    ScoreDB(const Path& path) : ScoreDB(path.u8string().c_str()) {}
    ~ScoreDB() override = default;
    ScoreDB(ScoreDB&) = delete;
    ScoreDB& operator= (ScoreDB&) = delete;

protected:
    void deleteScoreBMS(const char* tableName, const HashMD5& hash);
    [[nodiscard]] std::shared_ptr<ScoreBMS> getScoreBMS(const char* tableName, const HashMD5& hash) const;
    void updateScoreBMS(const char* tableName, const HashMD5& hash, const ScoreBMS& score);

public:
    void deleteChartScoreBMS(const HashMD5& hash);
    [[nodiscard]] std::shared_ptr<ScoreBMS> getChartScoreBMS(const HashMD5& hash) const;
    void updateChartScoreBMS(const HashMD5& hash, const ScoreBMS& score);

    void deleteCourseScoreBMS(const HashMD5& hash);
    [[nodiscard]] std::shared_ptr<ScoreBMS> getCourseScoreBMS(const HashMD5& hash) const;
    void updateCourseScoreBMS(const HashMD5& hash, const ScoreBMS& score);

    void preloadScore();

    [[nodiscard]] lunaticvibes::OverallStats getStats();

private:
    void updateStats(const ScoreBMS& score);
};
