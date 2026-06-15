#pragma once

#include "ButtonFactory.h"
#include "CafeEnvironmentFactory.h"
#include "CleanupZoneFactory.h"
#include "CoffeeMachineFactory.h"
#include "CupFactory.h"
#include "CustomerFactory.h"
#include "LiquidDropFactory.h"
#include "OrderIconFactory.h"
#include "PastryFactory.h"
#include "SpeechBubbleFactory.h"
#include <bagel.h>

namespace cafe
{
/** @brief Destroys an entity that has a PhysicsBody.
 *  Entity::destroy() alone leaks the b2Body — always use this helper. */
void destroyPhysicalEntity(bagel::ent_type id);
} // namespace cafe
