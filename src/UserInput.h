#pragma once

#include <SDL3/SDL.h>
#include <cstdint>
#include <vector>

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

/** @brief Per-frame SDL input state. keyDowns/keyUps collect ALL key events this frame. */
struct UserInput
{
    uint32_t                  controls{};
    SDL_FPoint                mousePos{};
    std::vector<SDL_Scancode> keyDowns{};
    std::vector<SDL_Scancode> keyUps{};
    uint8_t                   mouseButton{};
};

} // namespace cafe