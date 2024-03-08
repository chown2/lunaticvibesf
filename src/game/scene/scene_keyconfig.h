#pragma once
#include "scene.h"
#include "common/keymap.h"
#include <shared_mutex>

class SceneKeyConfig : public SceneBase
{
private:
    bool exiting = false;
    std::shared_mutex _mutex;

public:
    SceneKeyConfig();
    ~SceneKeyConfig() override;

protected:
    // Looper callbacks
    void _updateAsync() override;
    std::function<void()> _updateCallback;
    void updateStart();
    void updateMain();
    void updateFadeout();

protected:
    std::map<Input::Pad, long long> forceBargraphTriggerTimestamp;
    void updateForceBargraphs();

    void updateInfo(KeyMap k, int slot);
    void updateAllText();

    std::map<size_t, JoystickMask> joystickPrev;
    std::array<double, 2>   playerTurntableAngleAdd{ 0 };

protected:
    // Register to InputWrapper: judge / keysound
    void inputGamePress(InputMask&, const lunaticvibes::Time&);
    void inputGameAxis(double s1, double s2, const lunaticvibes::Time&);
    void inputGamePressKeyboard(KeyboardMask&, const lunaticvibes::Time&);
    void inputGamePressJoystick(JoystickMask&, size_t device, const lunaticvibes::Time&);
    void inputGameAbsoluteAxis(JoystickAxis&, size_t device, const lunaticvibes::Time&);

public:
    static void setInputBindingText(GameModeKeys keys, Input::Pad pad);
};