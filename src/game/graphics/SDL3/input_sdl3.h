#include <common/keymap.h>
#include <game/input/input_mgr.h>

#include <array>
#include <cstdint>
#include <shared_mutex>

#include <SDL3/SDL.h>

using int16_t = std::int16_t;

namespace sdl::state
{

inline constexpr size_t SDL_MOUSE_BUTTON_COUNT = 256;

inline std::shared_mutex g_input_mutex;
// `true` for keys currently being pressed down.
inline bool g_keyboard_scancodes[SDL_SCANCODE_COUNT];
// `true` for buttons currently being pressed down.
inline bool g_mouse_buttons[SDL_MOUSE_BUTTON_COUNT];
inline int g_mouse_x = 0;
inline int g_mouse_y = 0;
// Reset this to `0` after use.
inline short g_mouse_wheel_delta = 0;
inline std::array<std::array<int16_t, InputMgr::MAX_JOYSTICK_AXIS_COUNT>, InputMgr::MAX_JOYSTICK_COUNT> g_con_axes{};
// `true` for buttons currently being pressed down.
inline std::array<std::array<bool, InputMgr::MAX_JOYSTICK_BUTTON_COUNT>, InputMgr::MAX_JOYSTICK_COUNT> g_con_buttons{};

} // namespace sdl::state

unsigned char sdl_key_from_common_scancode(Input::Keyboard key);
