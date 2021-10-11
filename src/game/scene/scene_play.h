#pragma once
#include <variant>
#include <mutex>
#include <future>
#include "scene.h"
#include "scene_context.h"
#include "common/chartformat/chartformat.h"
#include "game/chart/chart.h"
#include "game/ruleset/ruleset_classic.h"

enum class ePlayMode
{
    SINGLE,
    LOCAL_BATTLE,
};

enum class ePlayState
{
    PREPARE,
    LOADING,
    LOAD_END,
    PLAYING,
    FAILED,
    FADEOUT,
};

class ScenePlay : public vScene
{
private:
	std::future<void> _loadChartFuture;

private:
    ePlayState _state;
    InputMask _inputAvailable;

private:
    ePlayMode _mode;
    std::vector<size_t> _currentKeySample;
    std::array <rc::gauge_ty, MAX_PLAYERS> _gaugetype{ rc::gauge_ty::GROOVE };

private:
    bool _isHoldingStart = false;
    bool _isHoldingSelect = false;
    bool _isExitingFromPlay = false;
    std::array<bool, 2> _isPlayerFinished{ false };
    std::array<int, 2> _ttAngleDelta{ 0 };

public:
    ScenePlay(ePlayMode);
	virtual ~ScenePlay()
	{
		sceneEnding = true; 
		if (_loadChartFuture.valid()) 
			_loadChartFuture.wait();
		loopEnd(); 
	}

private:
    std::array<size_t, 128> _bgmSampleIdxBuf{};
    std::array<size_t, 128> _keySampleIdxBuf{};

private:
	//std::map<size_t, std::variant<std::monostate, pVideo, pTexture>> _bgaIdxBuf{};
	//std::map<size_t, std::list<std::shared_ptr<SpriteVideo>>> _bgaVideoSprites{};	// set when loading skins, to bind videos while loading chart
	//size_t bgaBaseIdx = -1u;
	//size_t bgaLayerIdx = -1u;
	//size_t bgaPoorIdx = -1u;
	//pTexture bgaBaseTexture;
	//pTexture bgaLayerTexture;
	//pTexture bgaPoorTexture;
public:
	//void bindBgaVideoSprite(size_t idx, std::shared_ptr<SpriteVideo> pv) { _bgaVideoSprites[idx].push_back(pv); }

protected:
    // common
    void loadChart();
	constexpr double getWavLoadProgress() { return (_wavToLoad == 0) ? (_wavLoaded ? 1.0 : 0.0) : (double)_wavLoaded / _wavToLoad; }
	constexpr double getBgaLoadProgress() { return (_bmpToLoad == 0) ? (_wavLoaded ? 1.0 : 0.0) : (double)_bmpLoaded / _bmpToLoad; }

    void setInputJudgeCallback();
    void removeInputJudgeCallback();


protected:
    // loading indicators
    bool _chartLoaded = false;
    bool _rulesetLoaded = false;
    //bool _sampleLoaded = false;
    //bool _bgaLoaded = false;
    unsigned _wavLoaded = 0;
    unsigned _wavToLoad = 0;
    unsigned _bmpLoaded = 0;
    unsigned _bmpToLoad = 0;

protected:
    // Looper callbacks
    virtual void _updateAsync() override;
    void updatePrepare();
    void updateLoading();
    void updateLoadEnd();
    void updatePlaying();
    void updateFadeout();
    void updateFailed();

protected:
    // Inner-state updates
    void procCommonNotes();
    void playBGMSamples();
    void changeKeySampleMapping(Time t);
    void spinTurntable(bool startedPlaying);

protected:
    // Register to InputWrapper: judge / keysound
    void inputGamePress(InputMask&, Time);
    void inputGamePressJudge(InputMask&, Time);
    void inputGameHold(InputMask&, Time);
    void inputGameRelease(InputMask&, Time);
};