#pragma once

#include "WorldPos.h"

namespace cafe
{
class AssetManager;
class PhysicsContext;

/** Creates the pastry TV in the world: a static frame sprite plus a pastry-icon
 *  entity on top that pastryTvSystem cycles through the pastry types. The icon
 *  entity is also the pastry spawn button (created via createSpawnButton), so
 *  clicking the shown pastry spawns it. */
void createPastryTv(AssetManager& assets, PhysicsContext& physics, WorldPos pos);
} // namespace cafe