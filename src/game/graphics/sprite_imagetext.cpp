#include "sprite_imagetext.h"

#include <cassert>
#include <string>

#include "common/beat.h"
#include "common/encoding.h"

SpriteImageText::SpriteImageText(const SpriteImageTextBuilder& builder) : SpriteText(builder)
{
    _type = SpriteTypes::IMAGE_TEXT;
    _textures = builder.charTextures;
    assert(builder.charMappingList != nullptr);
    _chrList = builder.charMappingList;
    textHeight = builder.height;
    _margin = builder.margin;
}

void SpriteImageText::updateTextTexture(std::string&& text)
{
    // NOTE: we need to draw empty text too, to keep empty input fields selectable.

    _text = text;

    // convert UTF-8 to UTF-32
    std::u32string u32Text = utf8_to_utf32(text);

    // save characters
    float x = 0;
    float y = 0;
    float w = 0;
    _drawListOrig.clear();
    for (auto c : u32Text)
    {
        if (_chrList->find(c) != _chrList->end())
        {
            auto& r = _chrList->at(c).textureRect;
            if (c == U'\n')
            {
                x = 0;
                y += r.h;
                continue;
            }
            if (_chrList->at(c).textureIdx < _textures.size())
                _drawListOrig.push_back({ c, {x, y, (float)r.w, (float)r.h} });
            w = x + r.w;
            x += r.w + _margin;
        }
    }
    //_drawList = _drawListOrig;
    _drawRect = { 0, 0, (int)std::ceil(w), (int)textHeight};
}

void SpriteImageText::updateTextRect()
{
    _drawList = _drawListOrig;

    // size
    double sizeFactor = 1.0;
    if (_current.rect.h != _drawRect.h && _drawRect.h != 0)
    {
        sizeFactor = (double)_current.rect.h / _drawRect.h;
        for (auto& [c, r] : _drawList)
        {
            r.x *= sizeFactor;
            r.w *= sizeFactor;
            r.h *= sizeFactor;
        }
    }

    // shrink
    int text_w = static_cast<int>(std::round(_drawRect.w * sizeFactor));
    int rect_w = _current.rect.w * (double(_current.rect.h) / textHeight);
    if (text_w > rect_w)
    {
        double widthFactor = (double)rect_w / text_w;
        for (auto& [c, r] : _drawList)
        {
            r.x *= widthFactor;
            r.w *= widthFactor;
        }
        text_w = rect_w;
    }

    // align
    switch (align)
    {
    case TEXT_ALIGN_LEFT:
        break;
    case TEXT_ALIGN_CENTER:
        for (auto& [c, r] : _drawList) r.x -= text_w / 2;
        break;
    case TEXT_ALIGN_RIGHT:
        for (auto& [c, r] : _drawList) r.x -= text_w;
        break;
    }

    // move
    for (auto& [c, r] : _drawList)
    {
        r.x += _current.rect.x;
        r.y += _current.rect.y;
    }

    /*
    if (_haveParent && !_parent.expired())
    {
        auto parent = _parent.lock();
        auto r = parent->getCurrentRenderParams().rect;
        if (r.w == -1 && r.h == -1)
        {
            _current.rect.x = 0;
            _current.rect.y = 0;
        }
        else
        {
            _current.rect.x += parent->getCurrentRenderParams().rect.x;
            _current.rect.y += parent->getCurrentRenderParams().rect.y;
        }
    }
    */

}

bool SpriteImageText::update(const lunaticvibes::Time& t)
{
    _draw = updateMotion(t);
    if (_draw)
    {
        updateTextTexture(State::get(textInd));
        if (_draw) updateTextRect();
    }
    return _draw;
}

void SpriteImageText::draw() const
{
    if (isHidden()) return;

    if (_draw)
    {
        for (auto [c, r] : _drawList)
        {
            auto& [idx, rect] = _chrList->at(c);
            _textures[idx]->draw(rect, r, _current.color, _current.blend, _current.filter, _current.angle);
        }
    }
}
