#pragma once
#include "scene.h"

class SceneExitTrans : public SceneBase
{
public:
    SceneExitTrans();
    ~SceneExitTrans() override = default;

protected:
    // Looper callbacks
    void _updateAsync() override {}
};