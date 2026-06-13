#pragma once

#include "Ingredient.h"
#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
class AssetManager;

/** @brief Dynamic liquid drop of @p kind. Falls under gravity, collides with cup walls. */
bagel::Entity createLiquidDrop(AssetManager& assets, WorldPos pos, Ingredient kind);
} // namespace cafe
