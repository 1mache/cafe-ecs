#pragma once

#include "Ingredient.h"
#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
class AssetManager;

/** @brief Dynamic liquid drop of @p kind. Falls under gravity, collides with cup
 *  walls, destroyed on sensor entry. */
bagel::Entity createLiquidDrop(AssetManager& assets, WorldPos pos, Ingredient kind);

/** @brief Test-only: same as createLiquidDrop but without a texture (skips Assets). */
bagel::Entity createLiquidDropHeadless(WorldPos pos, Ingredient kind = Ingredient::Coffee);
} // namespace cafe
