#pragma once

#include "Ingredient.h"
#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
class AssetManager;
class PhysicsContext;

/** @brief A coffee machine: a kinematic body plus one pour pipe per ingredient.
 *  Each pipe carries a LiquidSpawner; intentSystem sets its active flag from
 *  the held pour key (1/2/3) to pour that liquid. */
struct CoffeeMachine
{
    bagel::Entity body;
    bagel::Entity pipes[INGREDIENT_COUNT];
};

CoffeeMachine createCoffeeMachine(PhysicsContext& physics, AssetManager& assets, WorldPos pos);
} // namespace cafe