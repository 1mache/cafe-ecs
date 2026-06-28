#pragma once

#include "WorldPos.h"

namespace cafe
{
class AssetManager;
class PhysicsContext;

// --- Supply layout (tune positions in-editor; world units, negative y = down) ---
namespace supply
{
inline constexpr float CUP_SPAWN_X    = -8.f; // spawn x for cup
inline constexpr float PASTRY_SPAWN_X = 2.f;  // spawn x for pastry
inline constexpr float DROP_FROM_Y    = 6.5f; // start height, just above the screen

inline constexpr WorldPos COFFEE_MACHINE_POS = {-7.f, -1.f};
inline constexpr WorldPos CUP_BUTTON_POS     = {-8.5f, -4.7f};
inline constexpr WorldPos PASTRY_BUTTON_POS  = {8.5f, -4.7f};

// Ice machine: a gray placeholder square next to the coffee machine, with its
// spawn button on the machine face. The cube drops from the machine spout
// (ICE_SPAWN_*) onto the counter — not from above the screen like cup/pastry.
// All four are tunable; keep ICE_SPAWN_* aligned with ICE_MACHINE_POS.
inline constexpr WorldPos ICE_MACHINE_POS = {-2.5f, -1.0f};
inline constexpr WorldPos ICE_BUTTON_POS  = {-2.5f, -1.5f};
inline constexpr float    ICE_SPAWN_X     = -2.5f;
inline constexpr float    ICE_SPAWN_Y     = -1.0f;

// Microwave: placeholder square; pats are dragged in and spat out at MICROWAVE_SPAWN_POS.
// Both are tunable — adjust by observation so the spawn lands on the counter beside it.
inline constexpr WorldPos MICROWAVE_POS       = {1.0f, -1.0f};
inline constexpr WorldPos MICROWAVE_SPAWN_POS = {2.3f, -0.5f};
} // namespace supply

/** @brief Summons a cup/pastry into the first free supply slot when its button was
 *  clicked this frame (SpawnButton.justPressed, set by intentSystem). */
void supplyButtonSystem(AssetManager& assets, PhysicsContext& physics);
} // namespace cafe
