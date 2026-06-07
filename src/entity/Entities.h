#pragma once

#include "CleanupZoneFactory.h"
#include "CoffeeMachineFactory.h"
#include "CupFactory.h"
#include "LiquidDropFactory.h"
#include <bagel.h>

namespace cafe
{
/** @brief Destroys an entity that has a PhysicsBody.
 *  Entity::destroy() alone leaks the b2Body — always use this helper. */
void destroyPhysicalEntity(bagel::ent_type id);
} // namespace cafe
