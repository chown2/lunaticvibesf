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
    ~SceneDecide() override;

protected:
    // Looper callbacks
    void _updateAsync() override;
    std::function<void()> _updateCallback;
    void updateStart();
    void updateSkip();
    void updateCancel();

protected:
    // Register to InputWrapper: judge / keysound
    void inputGamePress(InputMask&, const lunaticvibes::Time&);
    void inputGameHold(InputMask&, const lunaticvibes::Time&);
    void inputGameRelease(InputMask&, const lunaticvibes::Time&);
};