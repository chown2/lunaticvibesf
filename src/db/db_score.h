#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "common/types.h"
#include "common/hash.h"
#include "db/db_conn.h"

class ScoreBase;
class ScoreBMS;

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
    std::shared_ptr<ScoreBMS> getScoreBMS(const char* tableName, const HashMD5& hash) const;
    void updateScoreBMS(const char* tableName, const HashMD5& hash, const ScoreBMS& score);

public:
    void deleteChartScoreBMS(const HashMD5& hash);
    std::shared_ptr<ScoreBMS> getChartScoreBMS(const HashMD5& hash) const;
    void updateChartScoreBMS(const HashMD5& hash, const ScoreBMS& score);

    void deleteCourseScoreBMS(const HashMD5& hash);
    std::shared_ptr<ScoreBMS> getCourseScoreBMS(const HashMD5& hash) const;
    void updateCourseScoreBMS(const HashMD5& hash, const ScoreBMS& score);

    void preloadScore();
};
