#pragma once

#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
/** @brief Kinematic coffee machine. Carries a CoffeeSpawner; toggle .active to pour. */
bagel::Entity createCoffeeMachine(WorldPos pos, WorldPos spoutOffset,
                                  SDL_Texture* tex, float texW, float texH);
} // namespace cafe
