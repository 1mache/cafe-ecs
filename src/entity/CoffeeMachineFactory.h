#pragma once

#include "Ingredient.h"
#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
class AssetManager;
class PhysicsContext;

bagel::Entity createCoffeeMachine(AssetManager& assets, PhysicsContext& physics, WorldPos pos);
} // namespace cafe