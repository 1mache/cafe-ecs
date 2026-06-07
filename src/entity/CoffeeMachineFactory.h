#pragma once

#include "WorldPos.h"
#include <bagel.h>

namespace cafe
{
/** @brief Kinematic coffee machine. Carries a CoffeeSpawner; toggle .active to pour. */
bagel::Entity createCoffeeMachine(WorldPos pos, WorldPos spoutOffset);
} // namespace cafe
