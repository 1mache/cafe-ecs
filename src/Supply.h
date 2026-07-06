#pragma once

#include "WorldPos.h"


// --- Supply layout (tune positions in-editor; world units, negative y = down) ---
namespace cafe::supply
{
inline constexpr float CUP_SPAWN_X    = -8.f; // spawn x for cup
inline constexpr float PASTRY_SPAWN_X = 2.f;  // spawn x for pastry
inline constexpr float DROP_FROM_Y    = 6.5f; // start height, just above the screen

inline constexpr WorldPos COFFEE_MACHINE_POS = {-5.5f, -1.05f};
inline constexpr WorldPos CUP_BUTTON_POS     = {-8.3f, -4.5f};
// (no PASTRY_BUTTON_POS: the pastry button is the TV icon, placed at PASTRY_TV_POS.)

// Ice machine: a gray placeholder square next to the coffee machine, with its
// spawn button on the machine face. The cube drops from the machine spout
// (ICE_SPAWN_*) onto the counter — not from above the screen like cup/pastry.
// All four are tunable; keep ICE_SPAWN_* aligned with ICE_MACHINE_POS.
inline constexpr WorldPos ICE_MACHINE_POS = {-8.6f, -0.855f };
inline constexpr WorldPos ICE_BUTTON_POS  = {-2.5f, -1.5f};
inline constexpr float    ICE_BUTTON_W_PX = 6.f;
inline constexpr float    ICE_BUTTON_H_PX = 8.f;

// Microwave: placeholder square; pats are dragged in and spat out at MICROWAVE_SPAWN_POS.
// Both are tunable — adjust by observation so the spawn lands on the counter beside it.
inline constexpr WorldPos MICROWAVE_POS       = {1.0f, -1.4f};
inline constexpr WorldPos MICROWAVE_SPAWN_POS = {2.3f, -0.5f};

// Pastry TV: top-left display that rotates through pastry types; the pastry
// button spawns whichever it currently shows. Tunable — nudge by observation.
inline constexpr WorldPos PASTRY_TV_POS = {8.3f, -2.4f};
} // namespace cafe::supply

