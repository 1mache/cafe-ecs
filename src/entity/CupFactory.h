#pragma once

#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
class AssetManager;
class PhysicsContext;

/** @brief Kinematic cup at @p pos. Three solid walls + one interior sensor. */
bagel::Entity createCup(AssetManager& assets, PhysicsContext& physics, WorldPos pos);
} // namespace cafe
