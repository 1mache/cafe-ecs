#pragma once

#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
class AssetManager;

/** @brief Kinematic cup at @p pos. Three solid walls + one interior sensor. */
bagel::Entity createCup(AssetManager& assets, WorldPos pos, int capacity = 50);

/** @brief Test-only: same as createCup but with explicit texture dimensions (skips Assets). */
bagel::Entity createCupHeadless(WorldPos pos, float texW, float texH, int capacity = 50);
} // namespace cafe
