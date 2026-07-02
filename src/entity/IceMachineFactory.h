#pragma once

#include "WorldPos.h"
#include <bagel.h>

namespace cafe
{
class AssetManager;
class PhysicsContext;

bagel::Entity createIceMachine(AssetManager& assets, PhysicsContext& physics, WorldPos pos);
} // namespace cafe
