#pragma once

#include "WorldPos.h"

namespace cafe
{
class AssetManager;
class PhysicsContext;

// --- Supply layout (tune positions in-editor; world units, negative y = down) ---
namespace supply
{
inline constexpr int   CUP_CAPACITY  = 50;

inline constexpr float CUP_SPAWN_X      = -8.f;  // spawn x for cup
inline constexpr float PASTRY_SPAWN_X   = 2.f;   // spawn x for pastry
inline constexpr float DROP_FROM_Y      = 6.5f;  // start height, just above the screen

inline constexpr WorldPos CUP_BUTTON     = { -8.5f, -4.7f };
inline constexpr WorldPos PASTRY_BUTTON  = {  8.5f, -4.7f };
} // namespace supply

/** @brief Summons a cup/pastry into the first free supply slot when its button was
 *  clicked this frame (SpawnButton.justPressed, set by intentSystem). */
void supplyButtonSystem(PhysicsContext& physics, AssetManager& assets);
} // namespace cafe
