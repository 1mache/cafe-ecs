#pragma once

#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
class AssetManager;

/** @brief Kinematic coffee machine. Carries a CoffeeSpawner; toggle .active to pour. */
bagel::Entity createCoffeeMachine(AssetManager& assets,
                                  WorldPos      pos,
                                  WorldPos      spoutOffset);
} // namespace cafe
