#pragma once
#include "AssetManager.h"
#include "WorldPos.h"
#include "bagel.h"

namespace cafe
{
class PhysicsContext;

bagel::Entity createPastry(PhysicsContext& physics, WorldPos pos, AssetManager& assets);
}