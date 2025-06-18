#include <common/assert.h>
#include <common/log.h>
#include <common/sysutil.h>
#include <common/u8.h>
#include <game/graphics/SDL2/graphics_SDL2.h>
#include <game/graphics/font.h>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <filesystem>
#include <functional>

struct TtfInitQuit
{
    TtfInitQuit()
    {
        LOG_INFO << "[TTF] Initializing SDL_ttf...";
        if (!TTF_Init())
            panic("SDL_ttf init failed", SDL_GetError());
        LOG_INFO << "[TTF] Initialized SDL_ttf " << SDL_TTF_MAJOR_VERSION << '.' << SDL_TTF_MINOR_VERSION << "."
                 << SDL_TTF_MICRO_VERSION;
    }
    ~TtfInitQuit()
    {
        LOG_INFO << "[TTF] De-initializing SDL_ttf...";
        TTF_Quit();
    };
};

static void maybe_init_ttf_font()
{
    static TtfInitQuit _;
    (void)_;
}

lunaticvibes::Font::Font(const std::filesystem::path& filePath, int ptsize, int faceIndex)
    : _path(lunaticvibes::cs(filePath.u8string()))
{
    maybe_init_ttf_font();
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props, TTF_PROP_FONT_CREATE_FILENAME_STRING, _path.c_str());
    SDL_SetFloatProperty(props, TTF_PROP_FONT_CREATE_SIZE_FLOAT, static_cast<float>(ptsize));
    SDL_SetNumberProperty(props, TTF_PROP_FONT_CREATE_FACE_NUMBER, faceIndex);
    pushAndWaitMainThreadTask<void>([&]() { _data = TTF_OpenFontWithProperties(props); });
    SDL_DestroyProperties(props);
    if (!_data)
    {
        LOG_WARNING << "[TTF] " << filePath << ": " << SDL_GetError();
    }
    else
    {
        _loaded = true;
    }
}

lunaticvibes::Font::~Font()
{
    if (!_loaded)
        return;
    pushAndWaitMainThreadTask<void>(std::bind_front(TTF_CloseFont, static_cast<TTF_Font*>(_data)));
}

std::shared_ptr<Texture> lunaticvibes::Font::build_texture(const char* text, const Color& c)
{
    LVF_DEBUG_ASSERT(IsMainThread());
    if (!_loaded)
        return nullptr;

    SDL_Surface* surfaceText =
        TTF_RenderText_Blended(static_cast<TTF_Font*>(_data), text, 0, std::bit_cast<SDL_Color>(c));
    std::shared_ptr<Texture> pTexture = std::make_shared<Texture>(surfaceText);
    SDL_DestroySurface(surfaceText);
    return pTexture;
}
