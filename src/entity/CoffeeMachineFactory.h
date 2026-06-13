#pragma once

#include "Ingredient.h"
#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
class AssetManager;

/** @brief A coffee machine: a kinematic body plus one pour pipe per ingredient.
 *  Each pipe carries a CoffeeSpawner and a PourIntent — write the pipe's
 *  PourIntent.active to pour that liquid. */
struct CoffeeMachine
{
    bagel::Entity body;
    bagel::Entity pipes[INGREDIENT_COUNT];
};

CoffeeMachine createCoffeeMachine(AssetManager& assets, WorldPos pos);
} // namespace cafe