#pragma once

#include "WorldPos.h"
#include <bagel.h>

namespace cafe
{
class AssetManager;
class PhysicsContext;

/** @brief Dynamic, draggable ice cube (a large coffee particle). Falls under
 *  gravity, collides with cup walls, is caught by the cup interior sensor. */
bagel::Entity createIceCube(AssetManager& assets, PhysicsContext& physics, WorldPos pos);
} // namespace cafe
