#pragma once
#include "scene.h"

class ScenePlayRetryTrans : public SceneBase
{
public:
    ScenePlayRetryTrans();
    ~ScenePlayRetryTrans() override = default;

protected:
    // Looper callbacks
    void _updateAsync() override {}
};