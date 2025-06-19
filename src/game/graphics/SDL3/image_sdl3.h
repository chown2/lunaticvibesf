#pragma once

#include <filesystem>

struct SDL_Surface;

namespace lunaticvibes
{

// Returns false on error
bool save_into_png(SDL_Surface* surface, const std::filesystem::path& path);

} // namespace lunaticvibes
