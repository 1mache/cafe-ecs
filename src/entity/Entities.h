#pragma once

#include "CafeEnvironmentFactory.h"
#include "CleanupZoneFactory.h"
#include "CoffeeMachineFactory.h"
#include "CupFactory.h"
#include "CustomerFactory.h"
#include "IceCubeFactory.h"
#include "IceMachineFactory.h"
#include "LiquidDropFactory.h"
#include "MicrowaveFactory.h"
#include "NapkinFactory.h"
#include "OrderIconFactory.h"
#include "PastryFactory.h"
#include "SpawnButtonFactory.h"
#include "SpeechBubbleFactory.h"
#include <bagel.h>

namespace cafe
{
/** @brief Destroys an entity that has a PhysicsBody.
 *  Entity::destroy() alone leaks the b2Body — always use this helper. */
void destroyPhysicalEntity(bagel::ent_type id);

/** @brief Destroys a delivered item immediately.
 *  If the item is a Cup, also destroys its liquid drops, ice cubes, and child
 *  sprites so no orphaned particles or bodies remain.
 *  For any other item type (e.g. pastry), destroys only the item itself. */
void destroyDeliveredItem(bagel::ent_type id);

/** @brief Destroys every live entity (and its physics body, if it has one).
 *  Tears down the whole ECS for a scene switch. Collects all entity ids first,
 *  then destroys them (deferred destruction — never modifies the ECS while
 *  iterating). */
void destroyAllGameEntities();

/** @brief Applies a physics impulse to bounce a draggable entity back toward
 *  its origin. No-op if the entity has no PhysicsBody or a invalid body. */
void rejectItem(bagel::Entity e);
} // namespace cafe
