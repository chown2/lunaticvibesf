#include <execution>
#include "scene.h"
#include "common/beat.h"
#include "game/data/data.h"
#include "game/skin/skin_mgr.h"

// prototype
vScene::vScene(eMode mode, unsigned rate, bool backgroundInput) :
    AsyncLooper(std::bind(&vScene::_updateAsync, this), rate), _input(1000, backgroundInput)
{
    _skin = SkinMgr::get(mode);
	Time t;

    //gNumbers.reset();
    //gSliders.reset();
    //gSwitches.reset();
    gTimers.reset();
    gTimers.set(eTimer::SCENE_START, t.norm());
    gTimers.set(eTimer::START_INPUT, t.norm() + _skin->info.timeIntro);

    _input.register_p("SKIN_MOUSE_CLICK", std::bind(&vScene::update_mouse, this, std::placeholders::_1, std::placeholders::_2));
}

vScene::~vScene() 
{
    _input.unregister_p("SKIN_MOUSE_CLICK");
    sceneEnding = true; 
    loopEnd(); 
}

void vScene::update()
{
    _skin->update();
    auto [x, y] = _input.getCursorPos();
    _skin->update_mouse(x, y);
}

void vScene::update_mouse(InputMask& m, Time t)
{
    if (m[Input::Pad::M1])
    {
        auto [x, y] = _input.getCursorPos();
        _skin->update_mouse_click(x, y);
    }
}

void vScene::draw() const
{
    _skin->draw();
}