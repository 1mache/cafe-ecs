#pragma once
#include "Components.h"
#include "WorldPos.h"
#include <bagel.h>

namespace cafe
{
/** @brief Dynamic coffee drop. Falls under gravity, collides with cup walls, destroyed on sensor entry. */
bagel::Entity createLiquidDrop(WorldPos pos);

/** @brief Kinematic cup at @p pos. Three solid walls + one interior sensor. */
bagel::Entity createCup(WorldPos pos, int capacity = 50);

/** @brief Test-only: same as createCup but with explicit texture dimensions (skips Assets). */
bagel::Entity createCupHeadless(WorldPos pos, float texW, float texH, int capacity = 50);

/** @brief Kinematic coffee machine. Carries a CoffeeSpawner; toggle .active to pour. */
bagel::Entity createCoffeeMachine(WorldPos pos, WorldPos spoutOffset);

/** @brief Static off-screen sensor that destroys spilled coffee drops. */
bagel::Entity createCleanupZone();

/** @brief Destroys an entity that has a PhysicsBody.
 *  Entity::destroy() alone leaks the b2Body — always use this helper. */
void destroyPhysicalEntity(bagel::ent_type id);
} // namespace cafe
