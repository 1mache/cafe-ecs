#pragma once

#include "ItemTypes.h"

namespace cafe
{
class AssetManager;
class PhysicsContext;

/** Advances the pastry TV: every PASTRY_TV_ROTATE_TIME seconds it shows the next
 *  pastry type in the queue (enum order, looping). */
void pastryTvSystem(AssetManager& assets, float dt);

/** Single scan: reads the pastry SpawnButton's justPressed and the PastryTv's
 *  current index (order of encounter doesn't matter, both entities always
 *  exist) and spawns a pastry of that type once the loop is done. */
void pastrySupplySystem(AssetManager& assets, PhysicsContext& physics);
} // namespace cafe