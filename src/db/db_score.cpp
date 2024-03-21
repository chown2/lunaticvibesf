#include "db_score.h"

#include <algorithm>
#include <any>
#include <cassert>
#include <exception>
#include <memory>
#include <unordered_map>
#include <vector>

#include <stdint.h>

#include "common/log.h"
#include "common/types.h"
#include "common/hash.h"
#include "db/db_conn.h"

static constexpr auto&& CREATE_SCORE_HISTORY_BMS_TABLE_STR =
"CREATE TABLE IF NOT EXISTS score_history_bms( "
"md5 TEXT NOT NULL, "
"notes INTEGER NOT NULL, "
"score INTEGER NOT NULL, "
"fast INTEGER NOT NULL, "
"slow INTEGER NOT NULL, "
"maxcombo INTEGER NOT NULL , "
"addtime INTEGER NOT NULL , "
"exscore INTEGER NOT NULL, "
"lamp INTEGER NOT NULL, "
"pgreat INTEGER NOT NULL, "
"great INTEGER NOT NULL, "
"good INTEGER NOT NULL, "
"bad INTEGER NOT NULL, "
"bpoor INTEGER NOT NULL, "
"miss INTEGER NOT NULL, "
"bp INTEGER NOT NULL, "
"cb INTEGER NOT NULL, "
"playedtime INTEGER NOT NULL, "
"replay TEXT PRIMARY KEY UNIQUE NOT NULL "
")";

// Combined score_bms and score_history_bms.
static constexpr auto&& CREATE_SCORE_CACHE_BMS_TABLE_STR =
"CREATE TABLE IF NOT EXISTS score_cache_bms( "
"md5 TEXT PRIMARY KEY UNIQUE NOT NULL, "
"notes INTEGER NOT NULL, "
"score INTEGER NOT NULL, "
"fast INTEGER NOT NULL, "
"slow INTEGER NOT NULL, "
"maxcombo INTEGER NOT NULL, "
"addtime INTEGER NOT NULL, "
"pc INTEGER NOT NULL, "
"clearcount INTEGER NOT NULL, "
"exscore INTEGER NOT NULL, "
"lamp INTEGER NOT NULL, "
"pgreat INTEGER NOT NULL, "
"great INTEGER NOT NULL, "
"good INTEGER NOT NULL, "
"bad INTEGER NOT NULL, "
"bpoor INTEGER NOT NULL, "
"miss INTEGER NOT NULL, "
"bp INTEGER NOT NULL, "
"cb INTEGER NOT NULL, "
"playedtime INTEGER NOT NULL, "
"replay TEXT NOT NULL "
")";

// TODO: rename to like 'legacy_pbs_bms'.
// TODO: remove default values.
static constexpr auto&& CREATE_SCORE_BMS_TABLE_STR =
"CREATE TABLE IF NOT EXISTS score_bms( "
"md5 TEXT PRIMARY KEY UNIQUE NOT NULL, "
"notes INTEGER NOT NULL, "
"score INTEGER NOT NULL, "
"rate REAL NOT NULL, " // TODO: remove column
"fast INTEGER NOT NULL, "
"slow INTEGER NOT NULL, "
"maxcombo INTEGER NOT NULL DEFAULT 0, "
"addtime INTEGER NOT NULL DEFAULT 0, "
"pc INTEGER NOT NULL DEFAULT 0, " // TODO: rename column
"clearcount INTEGER NOT NULL DEFAULT 0, "
"exscore INTEGER NOT NULL, "
"lamp INTEGER NOT NULL, "
"pgreat INTEGER NOT NULL, "
"great INTEGER NOT NULL, "
"good INTEGER NOT NULL, "
"bad INTEGER NOT NULL, "
"bpoor INTEGER NOT NULL, "
"miss INTEGER NOT NULL, "
"bp INTEGER NOT NULL, "
"cb INTEGER NOT NULL, "
"replay TEXT "
")";
static constexpr size_t SCORE_BMS_PARAM_COUNT = 21;
struct score_bms_all_params
{
    std::string md5;
    long long notes = 0;
    long long score = 0;
    double    rate = 0;
    long long fast = 0;
    long long slow = 0;
    long long maxcombo = 0;
    long long addtime = 0;
    long long pc = 0;
    long long clearcount = 0;
    long long exscore = 0;
    long long lamp = 0;
    long long pgreat = 0;
    long long great = 0;
    long long good = 0;
    long long bad = 0;
    long long kpoor = 0;
    long long miss = 0;
    long long bp = 0;
    long long cb = 0;
    std::string replay;

    score_bms_all_params(const std::vector<std::any>& queryResult)
    {
        try
        {
            md5 = ANY_STR(queryResult.at(0));
            notes = ANY_INT(queryResult.at(1));
            score = ANY_INT(queryResult.at(2));
            rate = ANY_REAL(queryResult.at(3));
            fast = ANY_INT(queryResult.at(4));
            slow = ANY_INT(queryResult.at(5));
            maxcombo = ANY_INT(queryResult.at(6));
            addtime = ANY_INT(queryResult.at(7));
            pc = ANY_INT(queryResult.at(8));
            clearcount = ANY_INT(queryResult.at(9));
            exscore = ANY_INT(queryResult.at(10));
            lamp = ANY_INT(queryResult.at(11));
            pgreat = ANY_INT(queryResult.at(12));
            great = ANY_INT(queryResult.at(13));
            good = ANY_INT(queryResult.at(14));
            bad = ANY_INT(queryResult.at(15));
            kpoor = ANY_INT(queryResult.at(16));
            miss = ANY_INT(queryResult.at(17));
            bp = ANY_INT(queryResult.at(18));
            cb = ANY_INT(queryResult.at(19));
            replay = ANY_STR(queryResult.at(20));
        }
        catch (std::out_of_range&)
        {
        }
    }
};
bool convert_score_bms(ScoreBMS& out, const std::vector<std::any>& in)
{
    if (in.size() < SCORE_BMS_PARAM_COUNT) return false;

    score_bms_all_params params(in);
    out.notes = params.notes;
    out.score = params.score;
    out.rate = params.rate;
    out.fast = params.fast;
    out.slow = params.slow;
    out.maxcombo = params.maxcombo;
    out.addtime = params.addtime;
    out.playcount = params.pc;
    out.clearcount = params.clearcount;
    out.exscore = params.exscore;
    out.lamp = (ScoreBMS::Lamp)params.lamp;
    out.pgreat = params.pgreat;
    out.great = params.great;
    out.good = params.good;
    out.bad = params.bad;
    out.kpoor = params.kpoor;
    out.miss = params.miss;
    out.bp = params.bp;
    out.combobreak = params.cb;
    out.replayFileName = params.replay;
    out.play_time = lunaticvibes::Time{0};
    return true;
}

static constexpr auto&& CREATE_SCORE_COURSE_BMS_TABLE_STR =
"CREATE TABLE IF NOT EXISTS score_course_bms( "
"md5 TEXT PRIMARY KEY UNIQUE NOT NULL, "
"notes INTEGER NOT NULL, "
"score INTEGER NOT NULL, "
"rate REAL NOT NULL, "
"fast INTEGER NOT NULL, "
"slow INTEGER NOT NULL, "
"maxcombo INTEGER NOT NULL DEFAULT 0, "
"addtime INTEGER NOT NULL DEFAULT 0, "
"pc INTEGER NOT NULL DEFAULT 0, "
"clearcount INTEGER NOT NULL DEFAULT 0, "
"exscore INTEGER NOT NULL, "
"lamp INTEGER NOT NULL, "
"pgreat INTEGER NOT NULL, "
"great INTEGER NOT NULL, "
"good INTEGER NOT NULL, "
"bad INTEGER NOT NULL, "
"bpoor INTEGER NOT NULL, "
"miss INTEGER NOT NULL, "
"bp INTEGER NOT NULL, "
"cb INTEGER NOT NULL, "
"replay TEXT "
")";

static constexpr auto&& CREATE_STATS_CACHE_TABLE_STR =
"CREATE TABLE IF NOT EXISTS stats( "
"id INTEGER PRIMARY KEY CHECK (id = 573), " // only one row, ever.
"play_count INTEGER NOT NULL, clear_count INTEGER NOT NULL, "
"pgreat INTEGER NOT NULL, great INTEGER NOT NULL, good INTEGER NOT NULL, bad INTEGER NOT NULL, poor INTEGER NOT NULL, "
"running_combo INTEGER NOT NULL, max_running_combo INTEGER NOT NULL, "
"playtime INTEGER NOT NULL "
")";

static constexpr auto&& INIT_STATS_CACHE_TABLE_STR =
"INSERT OR IGNORE INTO stats(id, play_count, clear_count, pgreat, great, good, bad, poor, running_combo, max_running_combo, playtime) "
"VALUES(573, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)";

ScoreDB::ScoreDB(const char* path): SQLite(path, "SCORE")
{
    if (exec(CREATE_SCORE_BMS_TABLE_STR) != SQLITE_OK)
    {
        LOG_FATAL << "[ScoreDB] Failed to create table 'score_bms': " << errmsg();
        abort();
    }
    if (exec(CREATE_SCORE_HISTORY_BMS_TABLE_STR) != SQLITE_OK)
    {
        LOG_FATAL << "[ScoreDB] Failed to create table 'score_history_bms': " << errmsg();
        abort();
    }
    if (exec(CREATE_SCORE_CACHE_BMS_TABLE_STR) != SQLITE_OK)
    {
        LOG_FATAL << "[ScoreDB] Failed to create table 'score_cache_bms': " << errmsg();
        abort();
    }
    if (exec(CREATE_SCORE_COURSE_BMS_TABLE_STR) != SQLITE_OK)
    {
        LOG_FATAL << "[ScoreDB] Failed to create table 'score_course_bms': " << errmsg();
        abort();
    }
    if (exec(CREATE_STATS_CACHE_TABLE_STR) != SQLITE_OK)
    {
        LOG_FATAL << "[ScoreDB] Failed to create table 'stats': " << errmsg();
        abort();
    }
    if (exec(INIT_STATS_CACHE_TABLE_STR) != SQLITE_OK)
    {
        LOG_FATAL << "[ScoreDB] Failed to initialize table 'stats': " << errmsg();
        abort();
    }

    {
        const auto resp = query("SELECT count(*) FROM score_cache_bms");
        assert(resp.size() == 1);
        if (ANY_INT(resp[0][0]) == 0)
            rebuildBmsPbCache();
    }
}

void ScoreDB::deleteLegacyScoreBMS(const char* tableName, const HashMD5& hash)
{
    int ret;

    const auto hashStr = hash.hexdigest();

    char sqlbuf[128] = { 0 };
    snprintf(static_cast<char*>(sqlbuf), sizeof(sqlbuf) - 1, "DELETE FROM %s WHERE md5=?", tableName);
    ret = exec(static_cast<char*>(sqlbuf), {hashStr});
    if (ret != SQLITE_OK)
    {
        LOG_ERROR << "[ScoreDB] Failed to delete legacy score: " << errmsg();
        return;
    }

    cache[tableName].erase(hashStr);
}

std::shared_ptr<ScoreBMS> ScoreDB::getScoreBMS(const char* tableName, const HashMD5& hash) const
{
    return cache[tableName][hash.hexdigest()];
}

void ScoreDB::updateLegacyScoreBMS(const char* tableName, const HashMD5& hash, const ScoreBMS& score)
{
    int ret;

    std::string hashStr = hash.hexdigest();

    auto pRecord = getScoreBMS(tableName, hash);
    if (pRecord)
    {
        auto record = *pRecord;

        if (score.exscore > record.exscore)
        {
            record.rate = score.rate;
            record.fast = score.fast;
            record.slow = score.slow;
            record.exscore = score.exscore;
            record.pgreat = score.pgreat;
            record.great = score.great;
            record.good = score.good;
            record.bad = score.bad;
            record.kpoor = score.kpoor;
            record.miss = score.miss;
            record.combobreak = score.combobreak;
            record.replayFileName = score.replayFileName;
        }
        else if (score.exscore == record.exscore)
        {
            if (score.maxcombo > record.maxcombo || score.bp < record.bp || (int)score.lamp >(int)record.lamp)
                record.replayFileName = score.replayFileName;
        }

        record.bp = std::min(record.bp, score.bp);
        record.clearcount = std::max(record.clearcount, score.clearcount);
        record.maxcombo = std::max(record.maxcombo, score.maxcombo);
        record.notes = score.notes;
        record.playcount = std::max(record.playcount, score.playcount);
        record.score = std::max(record.score, score.score);

        if ((int)score.lamp > (int)record.lamp)
        {
            record.lamp = score.lamp;
        }

        char sqlbuf[224] = { 0 };
        sprintf(sqlbuf, "UPDATE %s SET notes=?,score=?,rate=?,fast=?,slow=?,maxcombo=?,addtime=?,pc=?,clearcount=?,exscore=?,lamp=?,"
            "pgreat=?,great=?,good=?,bad=?,bpoor=?,miss=?,bp=?,cb=?,replay=? WHERE md5=?", tableName);
        ret = exec(sqlbuf,
            { record.notes, record.score, record.rate, record.fast, record.slow,
            record.maxcombo, score.addtime, record.playcount, record.clearcount, record.exscore, (int)record.lamp,
            record.pgreat, record.great, record.good, record.bad, record.kpoor, record.miss, record.bp, record.combobreak, record.replayFileName,
            hashStr });
        assert(ret == SQLITE_OK);
    }
    else
    {
        char sqlbuf[224] = { 0 };
        sprintf(sqlbuf, "INSERT INTO %s(md5,notes,score,rate,fast,slow,maxcombo,addtime,pc,clearcount,exscore,lamp,"
            "pgreat,great,good,bad,bpoor,miss,bp,cb,replay) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)", tableName);
        ret = exec(sqlbuf,
            { hashStr,
            score.notes, score.score, score.rate, score.fast, score.slow,
            score.maxcombo, score.addtime, score.playcount, score.clearcount, score.exscore, (int)score.lamp,
            score.pgreat, score.great, score.good, score.bad, score.kpoor, score.miss, score.bp, score.combobreak, score.replayFileName });
        assert(ret == SQLITE_OK);
    }

    char sqlbuf[96] = { 0 };
    sprintf(sqlbuf, "SELECT * FROM %s WHERE md5=?", tableName);
    auto result = query(sqlbuf, { hashStr });
    assert(!result.empty());
    const auto& r = result[0];
    auto scoreRefetched = std::make_shared<ScoreBMS>();
    convert_score_bms(*scoreRefetched, r);
    cache[tableName].insert_or_assign(hashStr, std::move(scoreRefetched));
}

void ScoreDB::deleteChartScoreBMS(const HashMD5& hash)
{
    deleteLegacyScoreBMS("score_bms", hash);

    const std::string hash_str = hash.hexdigest();
    int ret = exec("DELETE FROM score_history_bms WHERE md5=?", {hash_str});
    if (ret != SQLITE_OK)
    {
        LOG_ERROR << "[ScoreDB] Failed to delete score: " << errmsg();
        return;
    }
    cache["score_bms"].erase(hash_str);
}

std::shared_ptr<ScoreBMS> ScoreDB::getChartScoreBMS(const HashMD5& hash) const
{
    return getScoreBMS("score_bms", hash);
}

void ScoreDB::updateLegacyChartScoreBMS(const HashMD5& hash, const ScoreBMS& score)
{
    updateLegacyScoreBMS("score_bms", hash, score);
    rebuildBmsPbCache();
}

void ScoreDB::insertChartScoreBMS(const HashMD5& hash, const ScoreBMS& score)
{
    saveChartScoreBmsToHistory(hash, score);
    updateCachedChartPbBms(hash, score);
    updateStats(score);
}

static bool convertCachedPbBms(ScoreBMS& out, const std::vector<std::any>& score) noexcept
try
{
    if (score.size() < 19) return false;
    // md5 = ANY_STR(score[0]);
    out.notes = ANY_INT(score[1]);
    out.score = ANY_INT(score[2]);
    out.fast = ANY_INT(score[3]);
    out.slow = ANY_INT(score[4]);
    out.maxcombo = ANY_INT(score[5]);
    out.addtime = ANY_INT(score[6]);
    out.exscore = ANY_INT(score[7]);
    out.lamp = static_cast<ScoreBMS::Lamp>(ANY_INT(score[8]));
    out.pgreat = ANY_INT(score[9]);
    out.great = ANY_INT(score[10]);
    out.good = ANY_INT(score[11]);
    out.bad = ANY_INT(score[12]);
    out.kpoor = ANY_INT(score[13]);
    out.miss = ANY_INT(score[14]);
    out.bp = ANY_INT(score[15]);
    out.combobreak = ANY_INT(score[16]);
    out.play_time = ANY_INT(score[17]);
    out.replayFileName = ANY_STR(score[18]);

    out.rate = static_cast<double>(out.exscore) / static_cast<double>(out.notes * 2) * 100;
    return true;
}
catch (const std::exception& e)
{
    LOG_ERROR << e.what();
    return false;
}

[[nodiscard]] std::shared_ptr<ScoreBMS> ScoreDB::fetchCachedPbBMS(const HashMD5& hash) const
{
    auto out = std::make_shared<ScoreBMS>();
    const auto scores = query("SELECT "
                              "0,notes,score,fast,slow,maxcombo,addtime,exscore,lamp,pgreat,great,good,bad,bpoor,miss,"
                              "bp,cb,playedtime,replay "
                              "FROM score_cache_bms WHERE md5 = ?",
                              {hash.hexdigest()});
    if (scores.empty())
        return nullptr;
    assert(scores.size() == 1);
    convertCachedPbBms(*out, scores[0]);
    return out;
}

void ScoreDB::saveChartScoreBmsToHistory(const HashMD5& hash, const ScoreBMS& score)
{
    const auto lamp = static_cast<int>(score.lamp);
    const auto play_time = score.play_time.norm();
    int ret = exec("INSERT INTO score_history_bms"
                   "(md5,notes,score,fast,slow,maxcombo,addtime,exscore,lamp,pgreat,great,good,bad,bpoor,miss,bp,"
                   "cb,playedtime,replay) "
                   "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                   {hash.hexdigest(), score.notes, score.score, score.fast, score.slow, score.maxcombo, score.addtime,
                    score.exscore, lamp, score.pgreat, score.great, score.good, score.bad, score.kpoor, score.miss,
                    score.bp, score.combobreak, play_time, score.replayFileName});
    assert(ret == SQLITE_OK);
}

void ScoreDB::updateCachedChartPbBms(const HashMD5& hash, const ScoreBMS& score)
{
    const std::string hashStr = hash.hexdigest();

    if (const std::shared_ptr<ScoreBMS> pRecord = fetchCachedPbBMS(hash); pRecord)
    {
        auto record = *pRecord;

        if (score.exscore > record.exscore)
        {
            record.rate = score.rate;
            record.fast = score.fast;
            record.slow = score.slow;
            record.exscore = score.exscore;
            record.pgreat = score.pgreat;
            record.great = score.great;
            record.good = score.good;
            record.bad = score.bad;
            record.kpoor = score.kpoor;
            record.miss = score.miss;
            record.combobreak = score.combobreak;
            record.replayFileName = score.replayFileName;
        }
        else if (score.exscore == record.exscore)
        {
            if (score.maxcombo > record.maxcombo || score.bp < record.bp || (int)score.lamp > (int)record.lamp)
                record.replayFileName = score.replayFileName;
        }

        record.bp = std::min(record.bp, score.bp);
        record.clearcount += score.clearcount;
        record.maxcombo = std::max(record.maxcombo, score.maxcombo);
        record.notes = score.notes;
        record.playcount += score.playcount;
        record.score = std::max(record.score, score.score);

        if ((int)score.lamp > (int)record.lamp)
        {
            record.lamp = score.lamp;
        }

        record.play_time += score.play_time;

        const auto play_time = record.play_time.norm();
        exec("UPDATE score_cache_bms SET "
             "notes=?,score=?,fast=?,slow=?,maxcombo=?,addtime=?,pc=?,clearcount=?,exscore=?,lamp=?,pgreat=?,"
             "great=?,good=?,bad=?,bpoor=?,miss=?,bp=?,cb=?,playedtime=?,replay=? WHERE md5=?",
             {record.notes,   record.score,     record.fast,       record.slow,    record.maxcombo,
              record.addtime, record.playcount, record.clearcount, record.exscore, (int)record.lamp,
              record.pgreat,  record.great,     record.good,       record.bad,     record.kpoor,
              record.miss,    record.bp,        record.combobreak, play_time,      record.replayFileName,
              hashStr});
        cache["score_bms"].insert_or_assign(hashStr, std::make_shared<ScoreBMS>(std::move(record)));
    }
    else
    {
        const auto play_time = score.play_time.norm();
        exec("INSERT INTO score_cache_bms"
             "(md5,notes,score,fast,slow,maxcombo,addtime,pc,clearcount,exscore,lamp,pgreat,great,good,bad,bpoor,"
             "miss,bp,cb,playedtime,replay) "
             "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
             {hashStr,          score.notes,     score.score,         score.fast,    score.slow,      score.maxcombo,
              score.addtime,    score.playcount, score.clearcount,    score.exscore, (int)score.lamp, score.pgreat,
              score.great,      score.good,      score.bad,           score.kpoor,   score.miss,      score.bp,
              score.combobreak, play_time,       score.replayFileName});
        cache["score_bms"].insert_or_assign(hashStr, std::make_shared<ScoreBMS>(score));
    }
}

static bool convertHistoryScoreBms(ScoreBMS& out, const std::vector<std::any>& score) noexcept
try
{
    if (score.size() < 19) return false;
    // md5 = ANY_STR(score[0]);
    out.notes = ANY_INT(score[1]);
    out.score = ANY_INT(score[2]);
    out.fast = ANY_INT(score[3]);
    out.slow = ANY_INT(score[4]);
    out.maxcombo = ANY_INT(score[5]);
    out.addtime = ANY_INT(score[6]);
    out.exscore = ANY_INT(score[7]);
    out.lamp = static_cast<ScoreBMS::Lamp>(ANY_INT(score[8]));
    out.pgreat = ANY_INT(score[9]);
    out.great = ANY_INT(score[10]);
    out.good = ANY_INT(score[11]);
    out.bad = ANY_INT(score[12]);
    out.kpoor = ANY_INT(score[13]);
    out.miss = ANY_INT(score[14]);
    out.bp = ANY_INT(score[15]);
    out.combobreak = ANY_INT(score[16]);
    out.play_time = ANY_INT(score[17]);
    out.replayFileName = ANY_STR(score[18]);

    out.rate = static_cast<double>(out.exscore) / static_cast<double>(out.notes * 2) * 100;
    return true;
}
catch (const std::exception& e)
{
    LOG_ERROR << e.what();
    return false;
}

void ScoreDB::rebuildBmsPbCache()
{
    LOG_DEBUG << "[ScoreDB] Asked to rebuild BMS score PB cache";
    transactionStart();
    exec("DELETE FROM score_cache_bms");
    for (const auto& raw_score : query("SELECT * FROM score_bms ORDER BY addtime"))
    {
        auto score = std::make_shared<ScoreBMS>();
        const std::string md5 = ANY_STR(raw_score[0]);
        convert_score_bms(*score, raw_score);
        updateCachedChartPbBms(md5, *score);
    }
    for (const auto& raw_score : query("SELECT "
                                       "md5,notes,score,fast,slow,maxcombo,addtime,exscore,lamp,pgreat,great,good,bad,"
                                       "bpoor,miss,bp,cb,playedtime,replay "
                                       "FROM score_history_bms "
                                       "ORDER BY addtime"))
    {
        auto score = std::make_shared<ScoreBMS>();
        const std::string md5 = ANY_STR(raw_score[0]);
        convertHistoryScoreBms(*score, raw_score);
        updateCachedChartPbBms(md5, *score);
    }
    transactionStop();
    preloadScore();
}

void ScoreDB::deleteCourseScoreBMS(const HashMD5& hash)
{
    deleteLegacyScoreBMS("score_course_bms", hash);
}

std::shared_ptr<ScoreBMS> ScoreDB::getCourseScoreBMS(const HashMD5& hash) const
{
    return getScoreBMS("score_course_bms", hash);
}

void ScoreDB::updateCourseScoreBMS(const HashMD5& hash, const ScoreBMS& score)
{
    updateLegacyScoreBMS("score_course_bms", hash, score);
}

void ScoreDB::preloadScore()
{
    cache.clear();
    for (const auto& r : query("SELECT * FROM score_course_bms"))
    {
        auto score = std::make_shared<ScoreBMS>();
        convert_score_bms(*score, r);
        cache["score_course_bms"].insert_or_assign(ANY_STR(r[0]), std::move(score));
    }
    for (const auto& r : query("SELECT "
                               "md5,notes,score,fast,slow,maxcombo,addtime,exscore,lamp,pgreat,great,good,bad,bpoor,"
                               "miss,bp,cb,playedtime,replay "
                               "FROM score_cache_bms"))
    {
        auto score = std::make_shared<ScoreBMS>();
        convertCachedPbBms(*score, r);
        cache["score_bms"].insert_or_assign(ANY_STR(r[0]), std::move(score));
    }
}

lunaticvibes::OverallStats ScoreDB::getStats()
{
    lunaticvibes::OverallStats stats;
    const auto stats_cache_all = query("SELECT play_count, clear_count, pgreat, great, good, bad, poor, running_combo, "
                                       "max_running_combo, playtime FROM stats");
    assert(stats_cache_all.size() == 1);
    const auto& stats_cache = stats_cache_all[0];
    stats.play_count = ANY_INT(stats_cache[0]);
    stats.clear_count = ANY_INT(stats_cache[1]);
    stats.pgreat = ANY_INT(stats_cache[2]);
    stats.great = ANY_INT(stats_cache[3]);
    stats.good = ANY_INT(stats_cache[4]);
    stats.bad = ANY_INT(stats_cache[5]);
    stats.poor = ANY_INT(stats_cache[6]);
    stats.running_combo = ANY_INT(stats_cache[7]);
    stats.max_running_combo = ANY_INT(stats_cache[8]);
    stats.playtime = ANY_INT(stats_cache[9]);
    return stats;
}

void ScoreDB::updateStats(const ScoreBMS& score)
{
    int ret;
    auto stats = getStats();
    // TODO: how should NOPLAY be counted?
    if (score.lamp == ScoreBMS::Lamp::NOPLAY)
    {
        LOG_DEBUG << "[ScoreDB] not counting NOPLAY score in stats";
        return;
    }
    stats.play_count += 1;
    stats.clear_count += static_cast<int64_t>(score.lamp != ScoreBMS::Lamp::FAILED);
    stats.pgreat += score.pgreat;
    stats.great += score.great;
    stats.good += score.good;
    // FIXME: not sure about these.
    stats.bad += score.bad;
    stats.poor += score.kpoor + score.miss;
    const auto first_running_combo = stats.running_combo + score.first_max_combo;
    stats.running_combo = score.combobreak == 0 ? stats.running_combo + score.maxcombo : score.final_combo;
    stats.max_running_combo = std::max(
        {stats.max_running_combo, first_running_combo, static_cast<int64_t>(score.maxcombo), stats.running_combo});
    stats.playtime += score.play_time.norm();

    ret = exec("UPDATE stats SET play_count=?, clear_count=?, pgreat=?, great=?, good=?, bad=?, poor=?, "
               "running_combo=?, max_running_combo=?, playtime=? WHERE id = 573",
               {stats.play_count, stats.clear_count, stats.pgreat, stats.great, stats.good, stats.bad, stats.pgreat,
                stats.running_combo, stats.max_running_combo, stats.playtime});
    if (ret != SQLITE_OK)
    {
        LOG_ERROR << "[ScoreDB] Failed to write new stats: " << errmsg();
    }
}