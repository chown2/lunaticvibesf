#pragma once
#include <mutex>
#include "scene.h"

enum class eDecideState
{
    START,
    SKIP,
    CANCEL,
};

class SceneDecide : public SceneBase
{
private:
    eDecideState state;
    InputMask _inputAvailable;

public:
    SceneDecide();
    virtual ~SceneDecide();

protected:
    // Looper callbacks
    virtual void _updateAsync() override;
    std::function<void()> _updateCallback;
    void updateStart();
    void updateSkip();
    void updateCancel();

protected:
    // Register to InputWrapper: judge / keysound
    void inputGamePress(InputMask&, lunaticvibes::Time);
    void inputGameHold(InputMask&, lunaticvibes::Time);
    void inputGameRelease(InputMask&, lunaticvibes::Time);
};