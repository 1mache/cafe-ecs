#pragma once
#include "AssetManager.h"
#include "WorldPos.h"
#include "bagel.h"

namespace cafe
{
class PhysicsContext;

bagel::Entity createPastry(AssetManager& assets, PhysicsContext& physics, WorldPos pos);
}