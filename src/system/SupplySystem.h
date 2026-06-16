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
inline constexpr float DROP_FROM_Y   = 6.5f;  // start height, just above the screen
inline constexpr float DROP_DURATION = 1.1f;  // seconds for the cartoonish drop-in
inline constexpr float SLOT_RADIUS   = 1.6f;  // a slot counts as occupied within this distance

inline constexpr WorldPos CUP_SLOTS[]    = { { -8.f, -2.5f }, { -5.f, -2.5f }, { -2.f, -2.5f } };
inline constexpr WorldPos PASTRY_SLOTS[] = { {  2.f, -2.5f }, { 4.5f, -2.5f }, {  7.f, -2.5f } };
inline constexpr WorldPos CUP_BUTTON     = { -8.5f, -4.7f };
inline constexpr WorldPos PASTRY_BUTTON  = {  8.5f, -4.7f };
} // namespace supply

/** @brief Summons a cup/pastry into the first free supply slot when its button was
 *  clicked this frame (SpawnButton.justPressed, set by intentSystem). */
void supplyButtonSystem(PhysicsContext& physics, AssetManager& assets);

/** @brief Advances the cartoonish drop-in tween; makes items draggable on landing. */
void fallingSystem(float dtSeconds);
} // namespace cafe
