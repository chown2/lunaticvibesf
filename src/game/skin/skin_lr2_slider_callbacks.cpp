

#include "skin_lr2_slider_callbacks.h"

#include "game/data/timer.h"
#include "game/data/number.h"
#include "game/data/option.h"
#include "game/data/slider.h"
#include "game/data/switch.h"
#include "game/data/text.h"

#include "game/sound/sound_mgr.h"
#include "game/sound/sound_sample.h"

#include "game/scene/scene_context.h"

#include "config/config_mgr.h"

namespace lr2skin::slider
{

#pragma region slider type callbacks

void select_pos(double p)
{
    gSliders.set(eSlider::SELECT_LIST, p);
    // TODO set position
    // TODO play sound
}

void customize_scrollbar(double p)
{
    gSliders.set(eSlider::SKIN_CONFIG_OPTIONS, p);
    // TODO set position
}

void ir_ranking_scrollbar(double p)
{
    // no ir support right now
}


// TODO update sound effects for callbacks below

void eq(int idx, double p)
{
    int val = int(p * 24) - 12;
    p = (val + 12) / 24.0;
    gSliders.set(eSlider(idx + (int)eSlider::EQ0), p);
    gNumbers.set(eNumber(idx + (int)eNumber::EQ0), val);
}

void master_volume(double p)
{
    gSliders.set(eSlider::VOLUME_MASTER, p);
    gNumbers.set(eNumber::VOLUME_MASTER, int(std::round(p * 100)));
}

void key_volume(double p)
{
    gSliders.set(eSlider::VOLUME_KEY, p);
    gNumbers.set(eNumber::VOLUME_KEY, int(std::round(p * 100)));
}

void bgm_volume(double p)
{
    gSliders.set(eSlider::VOLUME_BGM, p);
    gNumbers.set(eNumber::VOLUME_BGM, int(std::round(p * 100)));
}

void master_fx(int idx, double p)
{
    gSliders.set(idx == 0 ? eSlider::FX0_P1 : eSlider::FX0_P2, p);
    gNumbers.set(idx == 0 ? eNumber::FX0_P1 : eNumber::FX0_P2, int(std::round(p * 100)));
}

void key_fx(int idx, double p)
{
    gSliders.set(idx == 0 ? eSlider::FX1_P1 : eSlider::FX1_P2, p);
    gNumbers.set(idx == 0 ? eNumber::FX1_P1 : eNumber::FX1_P2, int(std::round(p * 100)));
}

void bgm_fx(int idx, double p)
{
    gSliders.set(idx == 0 ? eSlider::FX2_P1 : eSlider::FX2_P2, p);
    gNumbers.set(idx == 0 ? eNumber::FX2_P1 : eNumber::FX2_P2, int(std::round(p * 100)));
}

void pitch(double p)
{
    int val = int(p * 24) - 12;
    p = (val + 12) / 24.0;
    gSliders.set(eSlider::PITCH, p);
    gNumbers.set(eNumber::PITCH, val);
}


#pragma endregion

std::function<void(double)> getSliderCallback(int type)
{
    using namespace lr2skin::slider;
    using namespace std::placeholders;
    switch (type)
    {
    case 1:
        return std::bind(select_pos, _1);

    case 7:
        return std::bind(customize_scrollbar, _1);

    case 8:
        return std::bind(ir_ranking_scrollbar, _1);

    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
        return std::bind(eq, type - 10, _1);

    case 17:
        return std::bind(master_volume, _1);
    case 18:
        return std::bind(key_volume, _1);
    case 19:
        return std::bind(bgm_volume, _1);

    case 20:
        return std::bind(master_fx, 0, _1);
    case 21:
        return std::bind(master_fx, 1, _1);
    case 22:
        return std::bind(key_fx, 0, _1);
    case 23:
        return std::bind(key_fx, 1, _1);
    case 24:
        return std::bind(bgm_fx, 0, _1);
    case 25:
        return std::bind(bgm_fx, 1, _1);
    case 26:
        return std::bind(pitch, _1);

    default:
        return [](double) {};
    }
}


}