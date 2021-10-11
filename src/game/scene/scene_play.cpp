#include <cassert>
#include <future>
#include <set>
#include "scene_play.h"
#include "scene_context.h"
#include "game/sound/sound_mgr.h"
#include "game/ruleset/ruleset_bms.h"
#include "common/chartformat/chartformat_bms.h"
#include "game/chart/chart_bms.h"
#include "game/graphics/sprite_video.h"
#include "config/config_mgr.h"
#include "common/log.h"
#include "common/sysutil.h"
#include "game/sound/sound_sample.h"

struct InputTimerSwitchMap{
    eTimer tm;
    eSwitch sw;
};

static const InputTimerSwitchMap InputGamePressMapSingle[] =
{
    { eTimer::S1L_DOWN, eSwitch::S1L_DOWN },
    { eTimer::S1R_DOWN, eSwitch::S1R_DOWN },
    { eTimer::K11_DOWN, eSwitch::K11_DOWN },
    { eTimer::K12_DOWN, eSwitch::K12_DOWN },
    { eTimer::K13_DOWN, eSwitch::K13_DOWN },
    { eTimer::K14_DOWN, eSwitch::K14_DOWN },
    { eTimer::K15_DOWN, eSwitch::K15_DOWN },
    { eTimer::K16_DOWN, eSwitch::K16_DOWN },
    { eTimer::K17_DOWN, eSwitch::K17_DOWN },
    { eTimer::K18_DOWN, eSwitch::K18_DOWN },
    { eTimer::K19_DOWN, eSwitch::K19_DOWN },
    { eTimer::K1START_DOWN, eSwitch::K1START_DOWN },
    { eTimer::K1SELECT_DOWN, eSwitch::K1SELECT_DOWN },
    { eTimer::K1SPDUP_DOWN, eSwitch::K1SPDUP_DOWN },
    { eTimer::K1SPDDN_DOWN, eSwitch::K1SPDDN_DOWN },
    { eTimer::S2L_DOWN, eSwitch::S1L_DOWN },
    { eTimer::S2R_DOWN, eSwitch::S1R_DOWN },
    { eTimer::K21_DOWN, eSwitch::K11_DOWN },
    { eTimer::K22_DOWN, eSwitch::K12_DOWN },
    { eTimer::K23_DOWN, eSwitch::K13_DOWN },
    { eTimer::K24_DOWN, eSwitch::K14_DOWN },
    { eTimer::K25_DOWN, eSwitch::K15_DOWN },
    { eTimer::K26_DOWN, eSwitch::K16_DOWN },
    { eTimer::K27_DOWN, eSwitch::K17_DOWN },
    { eTimer::K28_DOWN, eSwitch::K18_DOWN },
    { eTimer::K29_DOWN, eSwitch::K19_DOWN },
    { eTimer::K2START_DOWN, eSwitch::K1START_DOWN },
    { eTimer::K2SELECT_DOWN, eSwitch::K1SELECT_DOWN },
    { eTimer::K2SPDUP_DOWN, eSwitch::K1SPDUP_DOWN },
    { eTimer::K2SPDDN_DOWN, eSwitch::K1SPDDN_DOWN },
};

static const InputTimerSwitchMap InputGameReleaseMapSingle[] =
{
    { eTimer::S1L_UP, eSwitch::S1L_DOWN },
    { eTimer::S1R_UP, eSwitch::S1R_DOWN },
    { eTimer::K11_UP, eSwitch::K11_DOWN },
    { eTimer::K12_UP, eSwitch::K12_DOWN },
    { eTimer::K13_UP, eSwitch::K13_DOWN },
    { eTimer::K14_UP, eSwitch::K14_DOWN },
    { eTimer::K15_UP, eSwitch::K15_DOWN },
    { eTimer::K16_UP, eSwitch::K16_DOWN },
    { eTimer::K17_UP, eSwitch::K17_DOWN },
    { eTimer::K18_UP, eSwitch::K18_DOWN },
    { eTimer::K19_UP, eSwitch::K19_DOWN },
    { eTimer::K1START_UP, eSwitch::K1START_DOWN },
    { eTimer::K1SELECT_UP, eSwitch::K1SELECT_DOWN },
    { eTimer::K1SPDUP_UP, eSwitch::K1SPDUP_DOWN },
    { eTimer::K1SPDDN_UP, eSwitch::K1SPDDN_DOWN },
    { eTimer::S2L_UP, eSwitch::S1L_DOWN },
    { eTimer::S2R_UP, eSwitch::S1R_DOWN },
    { eTimer::K21_UP, eSwitch::K11_DOWN },
    { eTimer::K22_UP, eSwitch::K12_DOWN },
    { eTimer::K23_UP, eSwitch::K13_DOWN },
    { eTimer::K24_UP, eSwitch::K14_DOWN },
    { eTimer::K25_UP, eSwitch::K15_DOWN },
    { eTimer::K26_UP, eSwitch::K16_DOWN },
    { eTimer::K27_UP, eSwitch::K17_DOWN },
    { eTimer::K28_UP, eSwitch::K18_DOWN },
    { eTimer::K29_UP, eSwitch::K19_DOWN },
    { eTimer::K2START_UP, eSwitch::K1START_DOWN },
    { eTimer::K2SELECT_UP, eSwitch::K1SELECT_DOWN },
    { eTimer::K2SPDUP_UP, eSwitch::K1SPDUP_DOWN },
    { eTimer::K2SPDDN_UP, eSwitch::K1SPDDN_DOWN },
};

static const InputTimerSwitchMap InputGamePressMapBattle[] =
{
    { eTimer::S1L_DOWN, eSwitch::S1L_DOWN },
    { eTimer::S1R_DOWN, eSwitch::S1R_DOWN },
    { eTimer::K11_DOWN, eSwitch::K11_DOWN },
    { eTimer::K12_DOWN, eSwitch::K12_DOWN },
    { eTimer::K13_DOWN, eSwitch::K13_DOWN },
    { eTimer::K14_DOWN, eSwitch::K14_DOWN },
    { eTimer::K15_DOWN, eSwitch::K15_DOWN },
    { eTimer::K16_DOWN, eSwitch::K16_DOWN },
    { eTimer::K17_DOWN, eSwitch::K17_DOWN },
    { eTimer::K18_DOWN, eSwitch::K18_DOWN },
    { eTimer::K19_DOWN, eSwitch::K19_DOWN },
    { eTimer::K1START_DOWN, eSwitch::K1START_DOWN },
    { eTimer::K1SELECT_DOWN, eSwitch::K1SELECT_DOWN },
    { eTimer::K1SPDUP_DOWN, eSwitch::K1SPDUP_DOWN },
    { eTimer::K1SPDDN_DOWN, eSwitch::K1SPDDN_DOWN },
    { eTimer::S2L_DOWN, eSwitch::S2L_DOWN },
    { eTimer::S2R_DOWN, eSwitch::S2R_DOWN },
    { eTimer::K21_DOWN, eSwitch::K21_DOWN },
    { eTimer::K22_DOWN, eSwitch::K22_DOWN },
    { eTimer::K23_DOWN, eSwitch::K23_DOWN },
    { eTimer::K24_DOWN, eSwitch::K24_DOWN },
    { eTimer::K25_DOWN, eSwitch::K25_DOWN },
    { eTimer::K26_DOWN, eSwitch::K26_DOWN },
    { eTimer::K27_DOWN, eSwitch::K27_DOWN },
    { eTimer::K28_DOWN, eSwitch::K28_DOWN },
    { eTimer::K29_DOWN, eSwitch::K29_DOWN },
    { eTimer::K2START_DOWN, eSwitch::K2START_DOWN },
    { eTimer::K2SELECT_DOWN, eSwitch::K2SELECT_DOWN },
    { eTimer::K2SPDUP_DOWN, eSwitch::K2SPDUP_DOWN },
    { eTimer::K2SPDDN_DOWN, eSwitch::K2SPDDN_DOWN },
};

static const InputTimerSwitchMap InputGameReleaseMapBattle[] =
{
    { eTimer::S1L_UP, eSwitch::S1L_DOWN },
    { eTimer::S1R_UP, eSwitch::S1R_DOWN },
    { eTimer::K11_UP, eSwitch::K11_DOWN },
    { eTimer::K12_UP, eSwitch::K12_DOWN },
    { eTimer::K13_UP, eSwitch::K13_DOWN },
    { eTimer::K14_UP, eSwitch::K14_DOWN },
    { eTimer::K15_UP, eSwitch::K15_DOWN },
    { eTimer::K16_UP, eSwitch::K16_DOWN },
    { eTimer::K17_UP, eSwitch::K17_DOWN },
    { eTimer::K18_UP, eSwitch::K18_DOWN },
    { eTimer::K19_UP, eSwitch::K19_DOWN },
    { eTimer::K1START_UP, eSwitch::K1START_DOWN },
    { eTimer::K1SELECT_UP, eSwitch::K1SELECT_DOWN },
    { eTimer::K1SPDUP_UP, eSwitch::K1SPDUP_DOWN },
    { eTimer::K1SPDDN_UP, eSwitch::K1SPDDN_DOWN },
    { eTimer::S2L_UP, eSwitch::S2L_DOWN },
    { eTimer::S2R_UP, eSwitch::S2R_DOWN },
    { eTimer::K21_UP, eSwitch::K21_DOWN },
    { eTimer::K22_UP, eSwitch::K22_DOWN },
    { eTimer::K23_UP, eSwitch::K23_DOWN },
    { eTimer::K24_UP, eSwitch::K24_DOWN },
    { eTimer::K25_UP, eSwitch::K25_DOWN },
    { eTimer::K26_UP, eSwitch::K26_DOWN },
    { eTimer::K27_UP, eSwitch::K27_DOWN },
    { eTimer::K28_UP, eSwitch::K28_DOWN },
    { eTimer::K29_UP, eSwitch::K29_DOWN },
    { eTimer::K2START_UP, eSwitch::K2START_DOWN },
    { eTimer::K2SELECT_UP, eSwitch::K2SELECT_DOWN },
    { eTimer::K2SPDUP_UP, eSwitch::K2SPDUP_DOWN },
    { eTimer::K2SPDDN_UP, eSwitch::K2SPDDN_DOWN },
};

////////////////////////////////////////////////////////////////////////////////

ScenePlay::ScenePlay(ePlayMode gamemode): vScene(gPlayContext.mode, 1000, true), _mode(gamemode)
{
    _currentKeySample.assign(Input::ESC, 0);

    _inputAvailable = INPUT_MASK_FUNC;
    _inputAvailable |= INPUT_MASK_1P | INPUT_MASK_2P;

    // file loading may delayed

    _state = ePlayState::PREPARE;

	if (gChartContext.chartObj == nullptr)
	{

	}

	// global info

    // basic info
    gTexts.set(eText::PLAY_TITLE, gChartContext.title);
    gTexts.set(eText::PLAY_SUBTITLE, gChartContext.title2);
    gTexts.set(eText::PLAY_ARTIST, gChartContext.artist);
    gTexts.set(eText::PLAY_SUBARTIST, gChartContext.artist2);
    gTexts.set(eText::PLAY_GENRE, gChartContext.genre);
    gNumbers.set(eNumber::PLAY_BPM, int(std::round(gChartContext.startBPM)));
    gNumbers.set(eNumber::BPM_MIN, int(std::round(gChartContext.minBPM)));
    gNumbers.set(eNumber::BPM_MAX, int(std::round(gChartContext.maxBPM)));

    // player datas

    // set gauge type
    if (gChartContext.chartObj)
        switch (gChartContext.chartObj->type())
        {
        case eChartFormat::BMS:
        case eChartFormat::BMSON:
            setTempInitialHealthBMS();
            break;
        default:
            break;
        }

    {
        using namespace std::string_literals;
        eGaugeOp tmp = eGaugeOp::GROOVE;
        switch (gPlayContext.mods[PLAYER_SLOT_1P].gauge)
        {
        case eModGauge::NORMAL:    tmp = eGaugeOp::GROOVE; break;
        case eModGauge::HARD:      tmp = eGaugeOp::SURVIVAL; break;
        case eModGauge::DEATH:     tmp = eGaugeOp::EX_SURVIVAL; break;
        case eModGauge::EASY:      tmp = eGaugeOp::GROOVE; break;
        case eModGauge::PATTACK:   tmp = eGaugeOp::EX_SURVIVAL; break;
        case eModGauge::GATTACK:   tmp = eGaugeOp::EX_SURVIVAL; break;
        case eModGauge::ASSISTEASY:tmp = eGaugeOp::GROOVE; break;
        case eModGauge::EXHARD:    tmp = eGaugeOp::EX_SURVIVAL; break;
        default: break;
        }
        _skin->setExtendedProperty("GAUGETYPE_1P"s, (void*)&tmp);

        if (_mode == ePlayMode::LOCAL_BATTLE)
        {
            switch (gPlayContext.mods[PLAYER_SLOT_2P].gauge)
            {
            case eModGauge::NORMAL:    tmp = eGaugeOp::GROOVE; break;
            case eModGauge::HARD:      tmp = eGaugeOp::SURVIVAL; break;
            case eModGauge::EASY:      tmp = eGaugeOp::GROOVE; break;
            case eModGauge::DEATH:     tmp = eGaugeOp::EX_SURVIVAL; break;
            case eModGauge::PATTACK:   tmp = eGaugeOp::EX_SURVIVAL; break;
            case eModGauge::GATTACK:   tmp = eGaugeOp::EX_SURVIVAL; break;
            case eModGauge::ASSISTEASY:tmp = eGaugeOp::GROOVE; break;
            case eModGauge::EXHARD:    tmp = eGaugeOp::EX_SURVIVAL; break;
            default: break;
            }
            _skin->setExtendedProperty("GAUGETYPE_2P"s, (void*)&tmp);
        }
    }

    using namespace std::placeholders;
    _input.register_p("SCENE_PRESS", std::bind(&ScenePlay::inputGamePress, this, _1, _2));
    _input.register_h("SCENE_HOLD", std::bind(&ScenePlay::inputGameHold, this, _1, _2));
    _input.register_r("SCENE_RELEASE", std::bind(&ScenePlay::inputGameRelease, this, _1, _2));

    loopStart();
    _input.loopStart();
}

void ScenePlay::setTempInitialHealthBMS()
{
    if (!gPlayContext.isCourse || gPlayContext.isCourseFirstStage)
    {
        switch (gPlayContext.mods[PLAYER_SLOT_1P].gauge)
        {
        case eModGauge::NORMAL:
        case eModGauge::EASY:
        case eModGauge::ASSISTEASY:
            gPlayContext.initialHealth[PLAYER_SLOT_1P] = 0.2;
            gNumbers.set(eNumber::PLAY_1P_GROOVEGAUGE, 20);
            break;

        case eModGauge::HARD:
        case eModGauge::DEATH:
        case eModGauge::PATTACK:
        case eModGauge::GATTACK:
        case eModGauge::EXHARD:
            gPlayContext.initialHealth[PLAYER_SLOT_1P] = 1.0;
            gNumbers.set(eNumber::PLAY_1P_GROOVEGAUGE, 100);
            break;

        default: break;
        }

        if (_mode == ePlayMode::LOCAL_BATTLE)
        {
            switch (gPlayContext.mods[PLAYER_SLOT_2P].gauge)
            {
            case eModGauge::NORMAL:
            case eModGauge::EASY:
            case eModGauge::ASSISTEASY:
                gPlayContext.initialHealth[PLAYER_SLOT_2P] = 0.2;
                gNumbers.set(eNumber::PLAY_2P_GROOVEGAUGE, 20);
                break;

            case eModGauge::HARD:
            case eModGauge::DEATH:
            case eModGauge::PATTACK:
            case eModGauge::GATTACK:
            case eModGauge::EXHARD:
                gPlayContext.initialHealth[PLAYER_SLOT_2P] = 1.0;
                gNumbers.set(eNumber::PLAY_2P_GROOVEGAUGE, 100);
                break;

            default: break;
            }
        }
    }
    else
    {
        gNumbers.set(eNumber::PLAY_1P_GROOVEGAUGE, (int)gPlayContext.initialHealth[0]);

        if (_mode == ePlayMode::LOCAL_BATTLE)
        {
            gNumbers.set(eNumber::PLAY_2P_GROOVEGAUGE, (int)gPlayContext.initialHealth[1]);
        }
    }

}

////////////////////////////////////////////////////////////////////////////////

void ScenePlay::loadChart()
{
    if (gChartContext.chartObj == nullptr || !gChartContext.chartObj->isLoaded())
    {
        if (gChartContext.path.empty())
        {
            LOG_ERROR << "[Play] Chart not specified!";
            return;
        }

        gChartContext.chartObj = vChartFormat::getFromFile(gChartContext.path);
    }

    if (gChartContext.chartObj == nullptr || !gChartContext.chartObj->isLoaded())
    {
        LOG_ERROR << "[Play] Invalid chart: " << gChartContext.path.string();
        return;
    }

    gChartContext.title = gChartContext.chartObj->title;
    gChartContext.title2 = gChartContext.chartObj->title2;
    gChartContext.artist = gChartContext.chartObj->artist;
    gChartContext.artist2 = gChartContext.chartObj->artist2;
    gChartContext.genre = gChartContext.chartObj->genre;
    gChartContext.minBPM = gChartContext.chartObj->minBPM;
    gChartContext.startBPM = gChartContext.chartObj->startBPM;
    gChartContext.maxBPM = gChartContext.chartObj->maxBPM;

    //load chart object from Chart object
    switch (gChartContext.chartObj->type())
    {
    case eChartFormat::BMS:
    {
        auto bms = std::reinterpret_pointer_cast<BMS>(gChartContext.chartObj);
        // TODO mods

        if (_mode == ePlayMode::LOCAL_BATTLE)
        {
            gPlayContext.chartObj[PLAYER_SLOT_1P] = std::make_shared<chartBMS>(bms);
            gPlayContext.chartObj[PLAYER_SLOT_2P] = std::make_shared<chartBMS>(bms);
        }
        else
        {
            gPlayContext.chartObj[PLAYER_SLOT_1P] = std::make_shared<chartBMS>(bms);
        }
        _chartLoaded = true;
        gNumbers.set(eNumber::PLAY_REMAIN_MIN, int(gPlayContext.chartObj[PLAYER_SLOT_1P]->getTotalLength().norm() / 1000 / 60));
        gNumbers.set(eNumber::PLAY_REMAIN_SEC, int(gPlayContext.chartObj[PLAYER_SLOT_1P]->getTotalLength().norm() / 1000 % 60));
        break;
    }

    case eChartFormat::BMSON:
    default:
		LOG_WARNING << "[Play] chart format not supported.";
        return;
    }

    gPlayContext.remainTime = gPlayContext.chartObj[PLAYER_SLOT_1P]->getTotalLength();

    // build Ruleset object
    switch (gPlayContext.rulesetType)
    {
    case eRuleset::BMS:
    {
        // set judge diff
        RulesetBMS::JudgeDifficulty judgeDiff;
        switch (gChartContext.chartObj->type())
        {
        case eChartFormat::BMS:
            switch (std::reinterpret_pointer_cast<BMS>(gChartContext.chartObj)->rank)
            {
            case 1: judgeDiff = RulesetBMS::JudgeDifficulty::HARD; break;
            case 2: judgeDiff = RulesetBMS::JudgeDifficulty::NORMAL; break;
            case 3: judgeDiff = RulesetBMS::JudgeDifficulty::EASY; break;
            case 4: judgeDiff = RulesetBMS::JudgeDifficulty::VERYEASY; break;
            case 6: judgeDiff = RulesetBMS::JudgeDifficulty::WHAT; break;
            case 0: 
            default:
                judgeDiff = RulesetBMS::JudgeDifficulty::VERYHARD; break;
            }
        case eChartFormat::BMSON:
        default: 
			LOG_WARNING << "[Play] chart format not supported.";
			break;
        }

        unsigned keys = 7;
        switch (gPlayContext.mode)
        {
        case eMode::PLAY5: 
        case eMode::PLAY5_2: keys = 5; break;
        case eMode::PLAY7:
        case eMode::PLAY7_2: keys = 7; break;
        case eMode::PLAY9:
        case eMode::PLAY9_2: keys = 9; break;
        case eMode::PLAY10: keys = 10; break;
        case eMode::PLAY14: keys = 14; break;
        defualt: break;
        }

        if (keys == 10 || keys == 14)
        {
            gPlayContext.ruleset[PLAYER_SLOT_1P] = std::make_shared<RulesetBMS>(
                gChartContext.chartObj, gPlayContext.chartObj[PLAYER_SLOT_1P],
                gPlayContext.mods[PLAYER_SLOT_1P].gauge, keys, judgeDiff, 
                gPlayContext.initialHealth[PLAYER_SLOT_1P], RulesetBMS::PlaySide::DP);
        }
        else if (_mode == ePlayMode::LOCAL_BATTLE)
        {
            gPlayContext.ruleset[PLAYER_SLOT_1P] = std::make_shared<RulesetBMS>(
                gChartContext.chartObj,  gPlayContext.chartObj[PLAYER_SLOT_1P],
                gPlayContext.mods[PLAYER_SLOT_1P].gauge, keys, judgeDiff,
                gPlayContext.initialHealth[PLAYER_SLOT_1P], RulesetBMS::PlaySide::BATTLE_1P);

            gPlayContext.ruleset[PLAYER_SLOT_2P] = std::make_shared<RulesetBMS>(
                gChartContext.chartObj, gPlayContext.chartObj[PLAYER_SLOT_2P],
                gPlayContext.mods[PLAYER_SLOT_2P].gauge, keys, judgeDiff,
                gPlayContext.initialHealth[PLAYER_SLOT_2P], RulesetBMS::PlaySide::BATTLE_2P);
        }
        else
        {
            gPlayContext.ruleset[PLAYER_SLOT_1P] = std::make_shared<RulesetBMS>(
                gChartContext.chartObj, gPlayContext.chartObj[PLAYER_SLOT_1P],
                gPlayContext.mods[PLAYER_SLOT_1P].gauge, keys, judgeDiff,
                gPlayContext.initialHealth[PLAYER_SLOT_1P], RulesetBMS::PlaySide::SP);
        }
        _rulesetLoaded = true;
    }
    break;

    default:
        break;
    }

    // load samples
    if (!gChartContext.isSampleLoaded && !sceneEnding)
    {
        auto dtor = std::async(std::launch::async, [&]() {
            SetThreadName("Chart sound sample loading thread");

            auto _pChart = gChartContext.chartObj;
            auto chartDir = gChartContext.chartObj->getDirectory();
            LOG_DEBUG << "[Play] Load files from " << chartDir.c_str();
            for (const auto& it : _pChart->wavFiles)
            {
				if (sceneEnding) break;
                if (it.empty()) continue;
                ++_wavToLoad;
            }
            if (_wavToLoad == 0)
            {
                _wavLoaded = 1;
                gChartContext.isSampleLoaded = true;
                return;
            }
            for (size_t i = 0; i < _pChart->wavFiles.size(); ++i)
            {
				if (sceneEnding) break;
                const auto& wav = _pChart->wavFiles[i];
                if (wav.empty()) continue;
				Path pWav(wav);
				if (pWav.is_absolute())
					SoundMgr::loadKeySample(pWav, i);
				else
					SoundMgr::loadKeySample((chartDir / wav), i);
                ++_wavLoaded;
            }
            gChartContext.isSampleLoaded = true;
        });
    }
    else
    {
        gChartContext.isSampleLoaded = true;
    }

    // load bga
    if (gSwitches.get(eSwitch::SYSTEM_BGA) && !gChartContext.isBgaLoaded && !sceneEnding)
    {
        auto dtor = std::async(std::launch::async, [&]() {
            SetThreadName("Chart BGA loading thread");

            auto _pChart = gChartContext.chartObj;
			auto chartDir = gChartContext.chartObj->getDirectory();
            for (const auto& it : _pChart->bgaFiles)
            {
				if (sceneEnding) return;
                if (it.empty()) continue;
                ++_bmpToLoad;
            }
            if (_bmpToLoad == 0)
            {
                _bmpLoaded = 1;
                return;
            }
            for (size_t i = 0; i < _pChart->bgaFiles.size(); ++i)
            {
				if (sceneEnding) return;
                const auto& bmp = _pChart->bgaFiles[i];
                if (bmp.empty()) continue;


				Path pBmp(bmp);
				if (pBmp.is_absolute())
					gPlayContext.bgaTexture->addBmp(i, pBmp);
				else
					gPlayContext.bgaTexture->addBmp(i, chartDir / pBmp);

				/*
				if (fs::exists(bmp) && fs::is_regular_file(bmp) && pBmp.has_extension())
				{
					if (video_file_extensions.find(toLower(pBmp.extension().string())) != video_file_extensions.end())
					{
						if (int vi; (vi = getVideoSlot()) >= 0)
						{
							_video[vi].setVideo(pBmp);
							auto pv = std::make_shared<sVideo>(_video[vi]);
							_bgaIdxBuf[i] = pv;
							for (auto& bv : _bgaVideoSprites[i])
								bv->bindVideo(pv);
						}
					}
					else
					{
						_bgaIdxBuf[i] = std::make_shared<Texture>(Image(bmp.c_str()));
					}
				}
				*/

                ++_bmpLoaded;
            }
			gPlayContext.bgaTexture->setSlotFromBMS(*std::reinterpret_pointer_cast<chartBMS>(gPlayContext.chartObj[PLAYER_SLOT_1P]));
            gChartContext.isBgaLoaded = true;
        });
    }
}

void ScenePlay::setInputJudgeCallback()
{
    using namespace std::placeholders;
    if (_mode == ePlayMode::LOCAL_BATTLE)
    {
        if (gPlayContext.ruleset[PLAYER_SLOT_1P] != nullptr)
        {
            auto fp = std::bind(&vRuleset::updatePress, gPlayContext.ruleset[PLAYER_SLOT_1P], _1, _2);
            _input.register_p("JUDGE_PRESS", fp);
            auto fh = std::bind(&vRuleset::updateHold, gPlayContext.ruleset[PLAYER_SLOT_1P], _1, _2);
            _input.register_h("JUDGE_HOLD", fh);
            auto fr = std::bind(&vRuleset::updateRelease, gPlayContext.ruleset[PLAYER_SLOT_1P], _1, _2);
            _input.register_r("JUDGE_RELEASE", fr);
        }
        else
            LOG_ERROR << "[Play] Ruleset of 1P not initialized!";

        if (gPlayContext.ruleset[PLAYER_SLOT_2P] != nullptr)
        {
            auto fp = std::bind(&vRuleset::updatePress, gPlayContext.ruleset[PLAYER_SLOT_2P], _1, _2);
            _input.register_p("JUDGE_PRESS", fp);
            auto fh = std::bind(&vRuleset::updateHold, gPlayContext.ruleset[PLAYER_SLOT_2P], _1, _2);
            _input.register_h("JUDGE_HOLD", fh);
            auto fr = std::bind(&vRuleset::updateRelease, gPlayContext.ruleset[PLAYER_SLOT_2P], _1, _2);
            _input.register_r("JUDGE_RELEASE", fr);
        }
        else
            LOG_ERROR << "[Play] Ruleset of 2P not initialized!";
    }
    else // SINGLE or MULTI
    {
        if (gPlayContext.ruleset[PLAYER_SLOT_1P] != nullptr)
        {
            auto fp = std::bind(&vRuleset::updatePress, gPlayContext.ruleset[PLAYER_SLOT_1P], _1, _2);
            _input.register_p("JUDGE_PRESS", fp);
            auto fh = std::bind(&vRuleset::updateHold, gPlayContext.ruleset[PLAYER_SLOT_1P], _1, _2);
            _input.register_h("JUDGE_HOLD", fh);
            auto fr = std::bind(&vRuleset::updateRelease, gPlayContext.ruleset[PLAYER_SLOT_1P], _1, _2);
            _input.register_r("JUDGE_RELEASE", fr);
        }
        else
            LOG_ERROR << "[Play] Ruleset not initialized!";
    }
}

void ScenePlay::removeInputJudgeCallback()
{
    for (size_t i = 0; i < gPlayContext.ruleset.size(); ++i)
    {
        _input.unregister_p("JUDGE_PRESS");
        _input.unregister_h("JUDGE_HOLD");
        _input.unregister_r("JUDGE_RELEASE");
    }
}

////////////////////////////////////////////////////////////////////////////////

void ScenePlay::_updateAsync()
{
	gNumbers.set(eNumber::SCENE_UPDATE_FPS, getRate());
    switch (_state)
    {
    case ePlayState::PREPARE:
		gOptions.set(eOption::PLAY_SCENE_STAT, Option::SPLAY_PREPARE);
        updatePrepare();
        break;
    case ePlayState::LOADING:
        updateLoading();
        break;
    case ePlayState::LOAD_END:
        updateLoadEnd();
        break;
    case ePlayState::PLAYING:
        updatePlaying();
        break;
    case ePlayState::FADEOUT:
        updateFadeout();
        break;
    case ePlayState::FAILED:
        updateFailed();
        break;
    }
}

void ScenePlay::updatePrepare()
{
	auto t = Time();
    auto rt = t - gTimers.get(eTimer::SCENE_START);
    if (rt.norm() > _skin->info.timeIntro)
    {
		gOptions.set(eOption::PLAY_SCENE_STAT, Option::SPLAY_LOADING);
		_loadChartFuture = std::async(std::launch::async, std::bind(&ScenePlay::loadChart, this));
        _state = ePlayState::LOADING;
        LOG_DEBUG << "[Play] State changed to LOADING";
    }
}

void ScenePlay::updateLoading()
{
	auto t = Time();
    auto rt = t - gTimers.get(eTimer::_LOAD_START);

    gNumbers.set(eNumber::PLAY_LOAD_PROGRESS_SYS, int(_chartLoaded * 50 + _rulesetLoaded * 50) / 100);
    gNumbers.set(eNumber::PLAY_LOAD_PROGRESS_WAV, int(getWavLoadProgress() * 100) / 100);
    gNumbers.set(eNumber::PLAY_LOAD_PROGRESS_BGA, int(getBgaLoadProgress() * 100) / 100);
    gNumbers.set(eNumber::PLAY_LOAD_PROGRESS_PERCENT, int
        (int(_chartLoaded) * 50 + int(_rulesetLoaded) * 50 + 
        getWavLoadProgress() * 100 + getBgaLoadProgress() * 100) / 300);

    gBargraphs.set(eBargraph::MUSIC_LOAD_PROGRESS_SYS, int(_chartLoaded) * 0.5 + int(_rulesetLoaded) * 0.5);
    gBargraphs.set(eBargraph::MUSIC_LOAD_PROGRESS_WAV, getWavLoadProgress());
    gBargraphs.set(eBargraph::MUSIC_LOAD_PROGRESS_BGA, getBgaLoadProgress());
    gBargraphs.set(eBargraph::MUSIC_LOAD_PROGRESS, int
        (int(_chartLoaded) * 0.5 + int(_rulesetLoaded) * 0.5 +
        getWavLoadProgress() + getBgaLoadProgress()) / 3.0);

    if (_chartLoaded && 
        _rulesetLoaded &&
        gChartContext.isSampleLoaded && 
        (!gSwitches.get(eSwitch::SYSTEM_BGA) || gChartContext.isBgaLoaded) &&
		rt > _skin->info.timeMinimumLoad)
    {
		gOptions.set(eOption::PLAY_SCENE_STAT, Option::SPLAY_READY);
		if (gPlayContext.bgaTexture) gPlayContext.bgaTexture->reset();
        gTimers.set(eTimer::PLAY_READY, t.norm());
        _state = ePlayState::LOAD_END;
        LOG_DEBUG << "[Play] State changed to READY";
    }
}

void ScenePlay::updateLoadEnd()
{
	auto t = Time();
    auto rt = t - gTimers.get(eTimer::PLAY_READY);
    spinTurntable(false);
    if (rt > _skin->info.timeGetReady)
    {
		gOptions.set(eOption::PLAY_SCENE_STAT, Option::SPLAY_PLAYING);
        gTimers.set(eTimer::PLAY_START, t.norm());
        setInputJudgeCallback();
		gChartContext.started = true;
        _state = ePlayState::PLAYING;
        LOG_DEBUG << "[Play] State changed to PLAY_START";
    }
}

void pushGraphPoints()
{
    gPlayContext.graphGauge[PLAYER_SLOT_1P].push_back(gPlayContext.ruleset[PLAYER_SLOT_1P]->getData().health * 100);

    gPlayContext.graphScore[PLAYER_SLOT_1P].push_back(gPlayContext.ruleset[PLAYER_SLOT_1P]->getData().score2);

    if (gPlayContext.ruleset[PLAYER_SLOT_2P])
        gPlayContext.graphScore[PLAYER_SLOT_2P].push_back(gPlayContext.ruleset[PLAYER_SLOT_2P]->getData().score2);

    gPlayContext.graphScoreTarget.push_back(static_cast<int>(std::floor(
        gPlayContext.ruleset[PLAYER_SLOT_1P]->getCurrentMaxScore() * (0.01 * gNumbers.get(eNumber::DEFAULT_TARGET_RATE)))));
}

void ScenePlay::updatePlaying()
{
	auto t = Time();
	auto rt = t - gTimers.get(eTimer::PLAY_START);
    gTimers.set(eTimer::MUSIC_BEAT, int(1000 * (gPlayContext.chartObj[PLAYER_SLOT_1P]->getCurrentBeat() * 4.0)) % 1000);
    gPlayContext.bgaTexture->update(rt, false);

    if (_mode == ePlayMode::LOCAL_BATTLE)
    {
        gPlayContext.chartObj[PLAYER_SLOT_1P]->update(rt);
        gPlayContext.chartObj[PLAYER_SLOT_2P]->update(rt);
        gPlayContext.ruleset[PLAYER_SLOT_1P]->update(t);
        gPlayContext.ruleset[PLAYER_SLOT_2P]->update(t);
    }
    else
    {
        gPlayContext.chartObj[PLAYER_SLOT_1P]->update(rt);
        gPlayContext.ruleset[PLAYER_SLOT_1P]->update(t);
    }

    gNumbers.set(eNumber::PLAY_BPM, int(std::round(gPlayContext.chartObj[PLAYER_SLOT_1P]->getCurrentBPM())));
    // play time / remain time
    {
        auto startTime = rt - gTimers.get(eTimer::PLAY_START);
        auto playtime = rt.norm() / 1000;
        auto remaintime = gPlayContext.chartObj[PLAYER_SLOT_1P]->getTotalLength().norm() - playtime;
        gNumbers.set(eNumber::PLAY_MIN, int(playtime / 60));
        gNumbers.set(eNumber::PLAY_SEC, int(playtime % 60));
        gNumbers.set(eNumber::PLAY_REMAIN_MIN, int(remaintime / 60));
        gNumbers.set(eNumber::PLAY_REMAIN_SEC, int(remaintime % 60));
    }

    procCommonNotes();
    changeKeySampleMapping(rt);
	//updateBga();

    // charts
    if (rt.norm() / 500 >= gPlayContext.graphGauge[PLAYER_SLOT_1P].size())
    {
        auto& g = gPlayContext.graphGauge[PLAYER_SLOT_1P];
        auto& r = gPlayContext.ruleset[PLAYER_SLOT_1P];
        int h = r->getClearHealth() * 100;
        if (!g.empty())
        {
            int ch = r->getData().health * 100;
            if ((g.back() < h && ch > h) || (g.back() > h && ch < h))
            {
                // just insert an interim point, as for a game we don't need to be too precise
                g.push_back(h);
            }
        }

        pushGraphPoints();
    }

    // health check (-> to failed)
    if (!_isExitingFromPlay)
    {
        switch (_mode)
        {
        case ePlayMode::SINGLE:
        {
            //if (context_play.health[context_play.playerSlot] <= 0)
            if (gPlayContext.ruleset[PLAYER_SLOT_1P]->isFailed())
            {
                pushGraphPoints();
                gTimers.set(eTimer::FAIL_BEGIN, t.norm());
                gOptions.set(eOption::PLAY_SCENE_STAT, Option::SPLAY_FAILED);
                _isExitingFromPlay = true;
                _state = ePlayState::FAILED;
                SoundMgr::stopSamples();
                SoundMgr::stopKeySamples();
                SoundMgr::playSample(eSoundSample::SOUND_PLAYSTOP);
                for (size_t i = 0; i < gPlayContext.ruleset.size(); ++i)
                {
                    _input.unregister_p("SCENE_PRESS");
                }
                LOG_DEBUG << "[Play] State changed to PLAY_FAILED";
            }

            if (_isPlayerFinished[PLAYER_SLOT_1P] ^ gPlayContext.ruleset[PLAYER_SLOT_1P]->isFinished())
            {
                _isPlayerFinished[PLAYER_SLOT_1P] = true;
                gTimers.set(eTimer::PLAY_P1_FINISHED, t.norm());
                if (gPlayContext.ruleset[PLAYER_SLOT_1P]->getData().combo == gPlayContext.chartObj[PLAYER_SLOT_1P]->getNoteCount())
                    gTimers.set(eTimer::PLAY_FULLCOMBO_1P, t.norm());
            }
        }
        break;

        case ePlayMode::LOCAL_BATTLE:
        {
            if (gPlayContext.ruleset[PLAYER_SLOT_1P]->isFailed() &&
                gPlayContext.ruleset[PLAYER_SLOT_2P]->isFailed())
            {
                pushGraphPoints();
                gTimers.set(eTimer::FAIL_BEGIN, t.norm());
                gOptions.set(eOption::PLAY_SCENE_STAT, Option::SPLAY_FAILED);
                _isExitingFromPlay = true;
                _state = ePlayState::FAILED;
                SoundMgr::stopSamples();
                SoundMgr::stopKeySamples();
                SoundMgr::playSample(eSoundSample::SOUND_PLAYSTOP);
                for (size_t i = 0; i < gPlayContext.ruleset.size(); ++i)
                {
                    _input.unregister_p("SCENE_PRESS");
                }
                LOG_DEBUG << "[Play] State changed to PLAY_FAILED";
            }

            if (_isPlayerFinished[PLAYER_SLOT_1P] ^ gPlayContext.ruleset[PLAYER_SLOT_1P]->isFinished())
            {
                _isPlayerFinished[PLAYER_SLOT_1P] = true;
                gTimers.set(eTimer::PLAY_P1_FINISHED, t.norm());
                if (gPlayContext.ruleset[PLAYER_SLOT_1P]->getData().combo == gPlayContext.chartObj[PLAYER_SLOT_1P]->getNoteCount())
                    gTimers.set(eTimer::PLAY_FULLCOMBO_1P, t.norm());
            }
            if (_isPlayerFinished[PLAYER_SLOT_2P] ^ gPlayContext.ruleset[PLAYER_SLOT_2P]->isFinished())
            {
                _isPlayerFinished[PLAYER_SLOT_2P] = true;
                gTimers.set(eTimer::PLAY_P2_FINISHED, t.norm());
                if (gPlayContext.ruleset[PLAYER_SLOT_2P]->getData().combo == gPlayContext.chartObj[PLAYER_SLOT_2P]->getNoteCount())
                    gTimers.set(eTimer::PLAY_FULLCOMBO_2P, t.norm());
            }
        }
        break;

        default:
            break;
        }
    }

    spinTurntable(true);

    //last note check
    if (rt.norm() - gPlayContext.chartObj[PLAYER_SLOT_1P]->getTotalLength().norm() >= 0)
    {
        gTimers.set(eTimer::FADEOUT_BEGIN, t.norm());
        gOptions.set(eOption::PLAY_SCENE_STAT, Option::SPLAY_FADEOUT);
        _isExitingFromPlay = true;
        _state = ePlayState::FADEOUT;
    }
     
}

void ScenePlay::updateFadeout()
{
    auto t = Time();
    auto rt = t - gTimers.get(eTimer::SCENE_START);
    auto ft = t - gTimers.get(eTimer::FADEOUT_BEGIN);
    if (gChartContext.started)
        gTimers.set(eTimer::MUSIC_BEAT, int(1000 * (gPlayContext.chartObj[PLAYER_SLOT_1P]->getCurrentBeat() * 4.0)) % 1000);
    spinTurntable(gChartContext.started);
	gPlayContext.bgaTexture->update(rt, false);

    if (_isExitingFromPlay)
    {
        _isExitingFromPlay = false;
        removeInputJudgeCallback();

        bool cleared = false;
        switch (_mode)
        {
        case ePlayMode::SINGLE:
            if (gPlayContext.ruleset[PLAYER_SLOT_1P]->isCleared())
                cleared = true;
            break;
        case ePlayMode::LOCAL_BATTLE:
            if (gPlayContext.ruleset[PLAYER_SLOT_1P]->isCleared() &&
                gPlayContext.ruleset[PLAYER_SLOT_2P]->isCleared())
                cleared = true;
            break;
        default:
            break;
        }
        gSwitches.set(eSwitch::RESULT_CLEAR, cleared);
    }

    if (ft >= _skin->info.timeOutro)
    {
        loopEnd();
        _input.loopEnd();
        if (gChartContext.started)
            gNextScene = eScene::RESULT;
        else
            gNextScene = eScene::SELECT;
    }

}

void ScenePlay::updateFailed()
{
    auto t = Time();
    auto ft = t - gTimers.get(eTimer::FAIL_BEGIN);
    if (gChartContext.started)
        gTimers.set(eTimer::MUSIC_BEAT, int(1000 * (gPlayContext.chartObj[PLAYER_SLOT_1P]->getCurrentBeat() * 4.0)) % 1000);
    spinTurntable(gChartContext.started);

    //failed play finished, move to next scene. No fadeout
    if (ft.norm() >= _skin->info.timeFailed)
    {
        // TODO check quick retry (start+select / white+black)

        gTimers.set(eTimer::FADEOUT_BEGIN, t.norm());
        gOptions.set(eOption::PLAY_SCENE_STAT, Option::SPLAY_FADEOUT);
        _state = ePlayState::FADEOUT;
    }
}


void ScenePlay::procCommonNotes()
{
    assert(gPlayContext.chartObj[PLAYER_SLOT_1P] != nullptr);
    auto it = gPlayContext.chartObj[PLAYER_SLOT_1P]->noteBgmExpired.begin();
    size_t i;
    size_t max = std::min(_bgmSampleIdxBuf.size(), gPlayContext.chartObj[PLAYER_SLOT_1P]->noteBgmExpired.size());
    for (i = 0; i < max && it != gPlayContext.chartObj[PLAYER_SLOT_1P]->noteBgmExpired.end(); ++i, ++it)
    {
        //if ((it->index & 0xF0) == 0xE0)
        {
            // BGA
            /*
            switch (it->index)
            {
            case 0xE0: bgaBaseIdx =  (unsigned)std::get<long long>(it->value); break;
            case 0xE1: bgaLayerIdx = (unsigned)std::get<long long>(it->value); break;
            case 0xE2: bgaPoorIdx =  (unsigned)std::get<long long>(it->value); break;
            default: break;
            }
            */
        }
        //else
        {
            // BGM
            _bgmSampleIdxBuf[i] = (unsigned)std::get<long long>(it->value);
        }
    }
    // TODO also play keysound in auto

    SoundMgr::playKeySample(i, (size_t*)_bgmSampleIdxBuf.data());
}

void ScenePlay::changeKeySampleMapping(Time t)
{
    static const Time MIN_REMAP_INTERVAL{ 1000 };
    if (_mode == ePlayMode::LOCAL_BATTLE)
    {
        for (size_t i = 0; i < Input::S2L; ++i)
            if (_inputAvailable[i])
            {
                assert(gPlayContext.chartObj[PLAYER_SLOT_1P] != nullptr);
                auto chan = gPlayContext.chartObj[PLAYER_SLOT_1P]->getLaneFromKey((Input::Pad)i);
                if (chan.first == NoteLaneCategory::_) continue;
                auto note = gPlayContext.chartObj[PLAYER_SLOT_1P]->incomingNote(chan.first, chan.second);
                if (note->time - t > MIN_REMAP_INTERVAL) continue;
                _currentKeySample[i] = (size_t)std::get<long long>(note->value);
            }
        for (size_t i = Input::S2L; i < Input::ESC; ++i)
            if (_inputAvailable[i])
            {
                assert(gPlayContext.chartObj[PLAYER_SLOT_2P] != nullptr);
                auto chan = gPlayContext.chartObj[PLAYER_SLOT_2P]->getLaneFromKey((Input::Pad)i);
                if (chan.first == NoteLaneCategory::_) continue;
                auto note = gPlayContext.chartObj[PLAYER_SLOT_2P]->incomingNote(chan.first, chan.second);
                if (note->time - t > MIN_REMAP_INTERVAL) continue;
                _currentKeySample[i] = (size_t)std::get<long long>(note->value);
            }
    }
    else
    {
        for (auto i = 0; i < Input::ESC; ++i)
            if (_inputAvailable[i])
            {
                assert(gPlayContext.chartObj[PLAYER_SLOT_1P] != nullptr);
                auto chan = gPlayContext.chartObj[PLAYER_SLOT_1P]->getLaneFromKey((Input::Pad)i);
                if (chan.first == NoteLaneCategory::_) continue;
                auto note = gPlayContext.chartObj[PLAYER_SLOT_1P]->incomingNote(chan.first, chan.second);
                if (note->time - t > MIN_REMAP_INTERVAL) continue;
                _currentKeySample[i] = (size_t)std::get<long long>(note->value);
            }
    }
}

/*
void ScenePlay::updateBga()
{
	// base
	if (std::holds_alternative<pVideo>(_bgaIdxBuf[bgaBaseIdx]))
	{
		auto pv = std::get<pVideo>(_bgaIdxBuf[bgaBaseIdx]);
		bgaBaseTexture = nullptr;
		pv->startPlaying();
	}
	else if (std::holds_alternative<pTexture>(_bgaIdxBuf[bgaBaseIdx]))
	{
		bgaBaseTexture = std::get<pTexture>(_bgaIdxBuf[bgaBaseIdx]);
	}
	else
	{
		bgaBaseTexture = nullptr;
	}

	// layer
	if (std::holds_alternative<pVideo>(_bgaIdxBuf[bgaLayerIdx]))
	{
		auto pv = std::get<pVideo>(_bgaIdxBuf[bgaLayerIdx]);
		bgaLayerTexture = nullptr;
		pv->startPlaying();
	}
	else if (std::holds_alternative<pTexture>(_bgaIdxBuf[bgaLayerIdx]))
	{
		bgaLayerTexture = std::get<pTexture>(_bgaIdxBuf[bgaLayerIdx]);
	}
	else
	{
		bgaLayerTexture = nullptr;
	}

	// poor
	if (std::holds_alternative<pVideo>(_bgaIdxBuf[bgaPoorIdx]))
	{
		auto pv = std::get<pVideo>(_bgaIdxBuf[bgaPoorIdx]);
		bgaPoorTexture = nullptr;
		pv->startPlaying();
	}
	else if (std::holds_alternative<pTexture>(_bgaIdxBuf[bgaPoorIdx]))
	{
		bgaPoorTexture = std::get<pTexture>(_bgaIdxBuf[bgaPoorIdx]);
	}
	else
	{
		bgaPoorTexture = nullptr;
	}
}
*/

void ScenePlay::spinTurntable(bool startedPlaying)
{
    auto a = _ttAngleDelta;
    if (startedPlaying)
    {
        auto t = Time();
        auto rt = t - gTimers.get(eTimer::PLAY_START);
        for (auto& aa : a)
            aa += int(rt.norm() * 180 / 1000);
    }
    for (auto& aa : a)
        aa %= 360;
    gNumbers.set(eNumber::_ANGLE_TT_1P, a[0]);
    gNumbers.set(eNumber::_ANGLE_TT_2P, a[1]);
}

////////////////////////////////////////////////////////////////////////////////
// CALLBACK
void ScenePlay::inputGamePress(InputMask& m, Time t)
{
    using namespace Input;

    auto input = _inputAvailable & m;

    // individual keys
    size_t sampleCount = 0;
    for (size_t i = 0; i < ESC; ++i)
        if (input[i])
        {
            if (_currentKeySample[i])
                _keySampleIdxBuf[sampleCount++] = _currentKeySample[i];
            gTimers.set(InputGamePressMapSingle[i].tm, t.norm());
            gTimers.set(InputGameReleaseMapSingle[i].tm, TIMER_NEVER);
            gSwitches.set(InputGamePressMapSingle[i].sw, true);
        }

    if (input[S1L] || input[S1R])
    {
        gTimers.set(eTimer::S1_DOWN, t.norm());
        gTimers.set(eTimer::S1_UP, TIMER_NEVER);
        gSwitches.set(eSwitch::S1_DOWN, true);
    }
    if (input[K1SPDUP])
    {
        int hs = gNumbers.get(eNumber::HS_1P);
        if (hs < 900)
            gNumbers.set(eNumber::HS_1P, hs + 25);
    }

    if (input[K1SPDDN])
    {
        int hs = gNumbers.get(eNumber::HS_1P);
        if (hs > 25)
            gNumbers.set(eNumber::HS_1P, hs - 25);
    }

    SoundMgr::playKeySample(sampleCount, (size_t*)&_keySampleIdxBuf[0]);

    if (_mode == ePlayMode::LOCAL_BATTLE)
    {
        if (input[S2L] || input[S2R])
        {
            gTimers.set(eTimer::S2_DOWN, t.norm());
            gTimers.set(eTimer::S2_UP, TIMER_NEVER);
            gSwitches.set(eSwitch::S2_DOWN, true);
        }

        if (input[K2SPDUP])
        {
            int hs = gNumbers.get(eNumber::HS_2P);
            if (hs < 900)
                gNumbers.set(eNumber::HS_2P, hs + 25);
        }
        if (input[K2SPDDN])
        {
            int hs = gNumbers.get(eNumber::HS_2P);
            if (hs > 25)
                gNumbers.set(eNumber::HS_2P, hs - 25);
        }
    }
    else
    {
        if (input[S2L] || input[S2R])
        {
            gTimers.set(eTimer::S1_DOWN, t.norm());
            gTimers.set(eTimer::S1_UP, TIMER_NEVER);
            gSwitches.set(eSwitch::S1_DOWN, true);
        }

        if (input[K2SPDUP])
        {
            int hs = gNumbers.get(eNumber::HS_1P);
            if (hs < 900)
                gNumbers.set(eNumber::HS_1P, hs + 25);
        }
        if (input[K2SPDDN])
        {
            int hs = gNumbers.get(eNumber::HS_1P);
            if (hs > 25)
                gNumbers.set(eNumber::HS_1P, hs - 25);
        }
    }

    if (input[Input::ESC] || (input[Input::K1START] && input[Input::K1SELECT]) || (input[Input::K2START] && input[Input::K2SELECT]))
    {
        if (gChartContext.started)
        {
            _isExitingFromPlay = true;

            switch (_mode)
            {
            case ePlayMode::SINGLE:
            {
                if (!_isPlayerFinished[PLAYER_SLOT_1P])
                {
                    gPlayContext.ruleset[PLAYER_SLOT_1P]->fail();
                }
            }
            break;

            case ePlayMode::LOCAL_BATTLE:
            {
                if (!_isPlayerFinished[PLAYER_SLOT_1P])
                {
                    gPlayContext.ruleset[PLAYER_SLOT_1P]->fail();
                }
                if (!_isPlayerFinished[PLAYER_SLOT_2P])
                {
                    gPlayContext.ruleset[PLAYER_SLOT_2P]->fail();
                }
            }
            break;

            default:
                break;
            }

            pushGraphPoints();

        }

        gTimers.set(eTimer::FADEOUT_BEGIN, t.norm());
        gOptions.set(eOption::PLAY_SCENE_STAT, Option::SPLAY_FADEOUT);
        _state = ePlayState::FADEOUT;
    }
}

// CALLBACK
void ScenePlay::inputGameHold(InputMask& m, Time t)
{
	using namespace Input;
	for (size_t i = 0; i < ESC; ++i)
	{
		// TODO analog spin speed
		switch (i)
		{
		case S1L: _ttAngleDelta[PLAYER_SLOT_1P] -= 5; break;
		case S1R: _ttAngleDelta[PLAYER_SLOT_1P] += 5; break;
		case S2L: _ttAngleDelta[PLAYER_SLOT_2P] -= 5; break;
		case S2R: _ttAngleDelta[PLAYER_SLOT_2P] += 5; break;
		default: break;
		}
	}
}

// CALLBACK
void ScenePlay::inputGameRelease(InputMask& m, Time t)
{
    using namespace Input;
    auto input = _inputAvailable & m;

    size_t count = 0;
    for (size_t i = 0; i < Input::ESC; ++i)
        if (input[i])
        {
            gTimers.set(InputGamePressMapSingle[i].tm, TIMER_NEVER);
            gTimers.set(InputGameReleaseMapSingle[i].tm, t.norm());
            gSwitches.set(InputGameReleaseMapSingle[i].sw, false);

            // TODO stop sample playing while release in LN notes
        }


    if (input[S1L] || input[S1R])
    {
        gTimers.set(eTimer::S1_DOWN, TIMER_NEVER);
        gTimers.set(eTimer::S1_UP, t.norm());
        gSwitches.set(eSwitch::S1_DOWN, false);
    }

    if (_mode == ePlayMode::LOCAL_BATTLE)
    {
        if (input[S2L] || input[S2R])
        {
            gTimers.set(eTimer::S2_DOWN, t.norm());
            gTimers.set(eTimer::S2_UP, TIMER_NEVER);
            gSwitches.set(eSwitch::S2_DOWN, true);
        }
    }
    else
    {
        if (input[S2L] || input[S2R])
        {
            gTimers.set(eTimer::S1_DOWN, t.norm());
            gTimers.set(eTimer::S1_UP, TIMER_NEVER);
            gSwitches.set(eSwitch::S1_DOWN, true);
        }
    }
}