#pragma once

#include <memory>

#include <common/types.h>
#include <game/graphics/blend_mode.h> // IWYU pragma: export
#include <game/graphics/color.h>      // IWUY pragma: export
#include <game/graphics/point.h>      // IWUY pragma: export
#include <game/graphics/rect.h>       // IWUY pragma: export
#include <game/graphics/rectf.h>      // IWUY pragma: export

// SDL forward declarations. Don't include SDL itself to avoid leaking implementation details.
struct SDL_IOStream;
struct SDL_Renderer;
struct SDL_Surface;
struct SDL_Texture;

// global control pointer, do not modify
inline SDL_Renderer* gFrameRenderer;

////////////////////////////////////////////////////////////////////////////////
// Convert SDL_Surface into SDL_Texture with subarea specified.
class Texture
{
protected:
    std::shared_ptr<SDL_Texture> _texture;
    Rect textureRect;
    bool loaded = false;
    mutable bool _filtering_set = false; // Set on first 'draw' call.

    void maybe_set_filtering(bool) const;

public:
    virtual void draw(const Rect& srcRect, RectF dstRect, const Color c, const BlendMode blend, const bool filter,
                      const double angleInDegrees, const Point* center) const;
    void draw(const Rect& srcRect, RectF dstRect, const Color c, const BlendMode blend, const bool filter,
              const double angleInDegrees) const
    {
        draw(srcRect, dstRect, c, blend, filter, angleInDegrees, nullptr);
    }
    void draw(const Rect& srcRect, RectF dstRect, const Color c, const BlendMode blend, const bool filter,
              const double angleInDegrees, const Point& center) const
    {
        draw(srcRect, dstRect, c, blend, filter, angleInDegrees, &center);
    }

public:
    enum class PixelFormat
    {
        UNKNOWN,
        UNSUPPORTED,

        RGB24,
        BGR24,

        YV12, // 4:2:0 Y + V + U
        IYUV, // 4:2:0 Y + U + V aka I420
        YUY2, // Y0 + U0 + Y1 + V0 aka YUYV
        UYVY, // U0 + Y0 + V0 + Y1
        YVYU, // Y0 + V0 + Y1 + U0
    };

public:
    Texture(const SDL_Surface* pSurface);
    Texture(SDL_Texture* pTexture, int w, int h);
    Texture(const Color& color);
    Texture(int w, int h, PixelFormat fmt, bool target);
    virtual ~Texture() = default;

public:
    void* raw();
    [[nodiscard]] Rect getRect() const { return textureRect; }
    [[nodiscard]] bool isLoaded() const { return loaded; }
    int updateYUV(uint8_t* Y, int Ypitch, uint8_t* U, int Upitch, uint8_t* V, int Vpitch);
};
