#pragma once

#include "WorldPos.h"
#include <bagel.h>

namespace cafe
{
class AssetManager;
class PhysicsContext;

/** @brief Placeholder gray-square microwave with a static DropSpace sensor.
 *  Drag a pastry onto it (release-over) to heat it. No spawn button. */
bagel::Entity createMicrowave(AssetManager& assets, PhysicsContext& physics, WorldPos pos);
} // namespace cafe
