#pragma once

#include <utility>

#include "game/input/input_wrapper.h"
#include "common/chartformat/chartformat.h"
#include "game/chart/chart.h"
#include "common/beat.h"

enum class RulesetType
{
    SOUND_ONLY,
    BMS,
};

class RulesetBase
{
public:
    struct BasicData
    {
        lunaticvibes::Time play_time;
        double health = 1.0;

        double acc;         // 0.0 - 100.0
        double total_acc;   // 0.0 - 100.0

        unsigned combo;
        unsigned maxCombo;
        unsigned comboDisplay;  // for course
        unsigned maxComboDisplay;

        unsigned judge[32]; // judge count. reserved for implementation
    };

protected:
    std::shared_ptr<ChartFormatBase> _format;
    std::shared_ptr<ChartObjectBase> _chart;
    BasicData _basic = { 0 };
    double _minHealth;
    double _clearHealth;
    bool _failWhenNoHealth = false;

    bool _isCleared = false;
    bool _isFailed = false;
    bool _isAutoplay = false;

    bool _hasStartTime = false;
    lunaticvibes::Time _startTime;

    unsigned notesReached = 0;    // total notes reached. +1 when timestamp reached
    unsigned notesExpired = 0;    // total notes expired. +1 when timestamp+POOR reached; +1 for LN when tail timestamp (no +POOR) is reached

public:
    RulesetBase() : _basic{ 0 } {}
    RulesetBase(std::shared_ptr<ChartFormatBase> format, std::shared_ptr<ChartObjectBase> chart) :
        _format(std::move(format)), _chart(std::move(chart)), _basic{ 0 }{}
    virtual ~RulesetBase() = default;
public:
    virtual void updatePress(InputMask& pg, const lunaticvibes::Time& t) = 0;
    virtual void updateHold(InputMask& hg, const lunaticvibes::Time& t) = 0;
    virtual void updateRelease(InputMask& rg, const lunaticvibes::Time& t) = 0;
    virtual void updateAxis(double s1, double s2, const lunaticvibes::Time& t) = 0;
    virtual void update(const lunaticvibes::Time& t) = 0;
public:
    BasicData getData() const { return _basic; }
    double getClearHealth() const { return _clearHealth; }
    bool failWhenNoHealth() const { return _failWhenNoHealth; }

    virtual bool isNoScore() const { return false; }    // for quick esc
    virtual bool isFinished() const { return notesExpired >= getNoteCount(); }
    virtual bool isCleared() const { return _isCleared; }
    virtual bool isFailed() const { return _isFailed; }

    virtual unsigned getCurrentMaxScore() const = 0;
    virtual unsigned getMaxScore() const = 0;

    // Some games count one LN as two notes. Leave it virtual
    virtual unsigned getNoteCount() const = 0;

    // A LN may have more than one combo (head+tail=2 / rolling LN). Leave it virtual
    virtual unsigned getMaxCombo() const = 0;

    virtual void fail() { _isFailed = true; }

    virtual void updateGlobals() = 0;

    void setComboDisplay(unsigned combo) { _basic.comboDisplay = combo; _basic.maxComboDisplay = std::max(_basic.maxComboDisplay, _basic.combo + combo); }
    void setMaxComboDisplay(unsigned combo) { _basic.maxComboDisplay = combo; }
    void setStartTime(const lunaticvibes::Time& t) { _hasStartTime = true; _startTime = t; }
};
