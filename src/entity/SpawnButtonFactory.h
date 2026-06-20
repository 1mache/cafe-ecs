#pragma once

#include "DropType.h"
#include "WorldPos.h"
#include <bagel.h>

namespace cafe
{
class AssetManager;
class PhysicsContext;
/** @brief Creates a clickable counter button that summons `item` when pressed. */
bagel::Entity createSpawnButton(AssetManager& assets, PhysicsContext& physics, WorldPos pos, DropType item);
} // namespace cafe
