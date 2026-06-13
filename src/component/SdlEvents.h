#pragma once

#include <SDL3/SDL.h>
#include <bagel.h>
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

/** @brief Per-frame SDL input state. Payload fields keep the last event value seen this frame. */
struct SdlEvents
{
    uint32_t     controls{};
    SDL_FPoint   mousePos{};
    SDL_Scancode keyScancode{};
    uint8_t      mouseButton{};
};

inline bool isTriggeredEvent(bagel::ent_type ent, Controls control)
{
    return (bagel::Entity{ ent }.get<SdlEvents>().controls & (1u << static_cast<int>(control))) != 0u;
}

} // namespace cafe

template <> struct bagel::Storage<cafe::SdlEvents> final : bagel::NoInstance { using type = bagel::SparseStorage<cafe::SdlEvents>; };
