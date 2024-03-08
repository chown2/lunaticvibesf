#pragma once
#include "scene.h"

class ScenePlayCourseTrans : public SceneBase
{
public:
    ScenePlayCourseTrans();
    ~ScenePlayCourseTrans() override = default;

protected:
    // Looper callbacks
    void _updateAsync() override {}
};