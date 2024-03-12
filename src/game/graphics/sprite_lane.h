#pragma once
#include "sprite.h"
#include "game/chart/chart.h"
#include <vector>
#include <memory>

// Draw the whole lane (on screen) with only one sprite per key.
// Currently only handles normal notes. LN, mines or others are meant to be managed by other Sprite class.
// Note that an instance of this class handles ONE note lane. That is, a 7+1 chart needs 8 instances of this class.
class SpriteLaneVertical: public SpriteStatic
{
protected:
	chart::NoteLaneCategory _category;
    chart::NoteLaneIndex _index;
    int _noteAreaHeight = 500;  // used to calculate note speed
    double _basespd;
    double _hispeed;
    std::list<RectF> _outRect;
    unsigned _currentMeasure = 0;
    bool _autoNotes = false;

public:
    unsigned playerSlot;
    std::shared_ptr<SpriteAnimated> pNote;

public:
    struct SpriteLaneVerticalBuilder : SpriteStaticBuilder
    {
        int player = 0;
        bool autoNotes = false;
        double baseSpeed = 1.0;
        double laneSpeed = 1.0;
        chart::NoteLaneCategory laneCategory = chart::NoteLaneCategory::_;
        chart::NoteLaneIndex laneIndex = chart::NoteLaneIndex::_;

        std::shared_ptr<SpriteLaneVertical> build() const { return std::make_shared<SpriteLaneVertical>(*this); }
    };
public:
    SpriteLaneVertical() = delete;
    SpriteLaneVertical(const SpriteLaneVerticalBuilder& builder);
    ~SpriteLaneVertical() override = default;
    void buildNote(const SpriteAnimated::SpriteAnimatedBuilder& builder);

public:
    void setHeight(int h) { _noteAreaHeight = h; }
    void setMotionLoopTo(int t) override;
    void setMotionStartTimer(IndexTimer t) override;
    void appendMotionKeyFrame(const MotionKeyFrame& f) override;

    chart::NoteLane getLane() const;
    void getRectSize(int& w, int& h);
    bool update(const lunaticvibes::Time& t) override;
    virtual void updateNoteRect(const lunaticvibes::Time& t);
    void draw() const override;
    void adjustAfterUpdate(int x, int y, int w = 0, int h = 0) override;

protected:
    bool _hiddenCompatible = false;
    std::shared_ptr<Texture> _hiddenCompatibleTexture = nullptr;
    bool _hiddenCompatibleDraw = false;
    RectF _hiddenCompatibleArea;
public:
    void setHIDDENCompatible() { _hiddenCompatible = true; }
protected:
    void updateHIDDENCompatible();

};


class SpriteLaneVerticalLN : public SpriteLaneVertical
{
protected:
    std::list<RectF> _outRectBody, _outRectTail;
    bool headExpired = false;
    bool tailExpired = false;
    bool headHit = false;
    bool tailHit = false;
    bool animLimited = false;
public:
    std::shared_ptr<SpriteAnimated> pNoteBody, pNoteTail;

public:
    struct SpriteLaneVerticalLNBuilder : SpriteLaneVerticalBuilder
    {
        std::shared_ptr<SpriteLaneVerticalLN> build() { return std::make_shared<SpriteLaneVerticalLN>(*this); }
    };
public:
    SpriteLaneVerticalLN() = delete;
    SpriteLaneVerticalLN(const SpriteLaneVerticalLNBuilder& builder);
    ~SpriteLaneVerticalLN() override = default;
    void buildNoteHead(const SpriteAnimated::SpriteAnimatedBuilder& builder);
    void buildNoteBody(const SpriteAnimated::SpriteAnimatedBuilder& builder);
    void buildNoteTail(const SpriteAnimated::SpriteAnimatedBuilder& builder);

public:
    void setMotionStartTimer(IndexTimer t) override;

    void updateNoteRect(const lunaticvibes::Time& t) override;
    void draw() const override;
    void adjustAfterUpdate(int x, int y, int w = 0, int h = 0) override;
};