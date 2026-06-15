#pragma once

#include <SDL3/SDL.h>
#include <cstdint>

namespace cafe
{
enum class Controls
{
    Quit = 0,
    KeyDown,
    KeyUp,
    MouseButtonDown,
    MouseButtonUp,
    MouseMotion,
    count
};

constexpr uint32_t controlBit(Controls control)
{
    return 1u << static_cast<int>(control);
}

/** @brief Per-frame SDL input state. Payload fields keep the last event value seen this frame. */
struct UserInput
{
    uint32_t     controls{};
    SDL_FPoint   mousePos{};
    SDL_Scancode keyScancode{};
    uint8_t      mouseButton{};
};

} // namespace cafe