#pragma once

#include "scene.h"

#include <game/lua_ir.h>

#include <future>
#include <memory>

class SkinMgr;

class ScoreBase;
enum class eResultState
{
    DRAW,
    STOP,
    RECORD,
    FADEOUT,
    WAIT_ARENA,
};

class SceneResult : public SceneBase
{
public:
    explicit SceneResult(const std::shared_ptr<SkinMgr>& skinMgr);
    ~SceneResult() override;

protected:
    // Looper callbacks
    void _updateAsync() override;
    void updateDraw();
    void updateStop();
    void updateRecord();
    void updateFadeout();
    void updateWaitArena();

protected:
    // Register to InputWrapper: judge / keysound
    void inputGamePress(InputMask&, const lunaticvibes::Time&);
    void inputGameHold(InputMask&, const lunaticvibes::Time&);
    void inputGameRelease(InputMask&, const lunaticvibes::Time&);

private:
    eResultState state;
    InputMask _inputAvailable;

protected:
    std::future<std::optional<lunaticvibes::ir::ScoreResult>> _scoreUpload;
    bool _scoreSyncFinished = false;
    bool _retryRequested = false;
    bool _savedScore = false;
    std::shared_ptr<ScoreBase> _pScoreOld;

    bool saveScore = false;
    ScoreBMS::Lamp saveLampMax;
    ScoreBMS::Lamp lamp[2];
};
