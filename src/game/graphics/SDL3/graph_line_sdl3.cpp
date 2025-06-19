#include <common/log.h>
#include <game/graphics/graph_line.h>
#include <game/graphics/graphics.h>

#include <SDL3/SDL.h>

#include <cmath>

// FIXME: this draws ugly lines and was mostly written by an AI. It does not depend on SDL_gfx though!
static void draw_thick_line(SDL_Renderer* renderer, float x1, float y1, float x2, float y2, Uint8 width, Uint8 r,
                            Uint8 g, Uint8 b, Uint8 a)
{
    if (!SDL_SetRenderDrawColor(renderer, r, g, b, a))
    {
        LOG_ERROR << "[SDL3] SDL_SetRenderDrawColor error: " << SDL_GetError();
    }

    const float angle = std::atan2(static_cast<float>(y2 - y1), static_cast<float>(x2 - x1));
    const float cosAngle = std::cos(angle);
    const float sinAngle = std::sin(angle);

    for (int i = -width / 2; i <= width / 2; i++)
    {
        const float offsetX = static_cast<float>(i) * sinAngle;
        const float offsetY = static_cast<float>(i) * -cosAngle;
        if (!SDL_RenderLine(renderer, x1 + offsetX, y1 + offsetY, x2 + offsetX, y2 + offsetY))
        {
            LOG_ERROR << "[SDL3] SDL_RenderLine error: " << SDL_GetError();
        }
    }
}

void lunaticvibes::GraphLineDrawer::draw(Point p1, Point p2, Color c) const
{
    const int ss = lunaticvibes::window::graphics_get_supersample_level();
    draw_thick_line(gFrameRenderer, static_cast<float>(p1.x * ss), static_cast<float>(p1.y * ss),
                    static_cast<float>(p2.x * ss), static_cast<float>(p2.y * ss), _width * ss, c.r, c.g, c.b, c.a);
}
