#pragma once

#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
class AssetManager;

/** @brief Dynamic coffee drop. Falls under gravity, collides with cup walls, destroyed on sensor entry. */
bagel::Entity createLiquidDrop(AssetManager& assets, WorldPos pos);

/** @brief Test-only: same as createLiquidDrop but without a texture (skips Assets). */
bagel::Entity createLiquidDropHeadless(WorldPos pos);
} // namespace cafe
