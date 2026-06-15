#pragma once

#include "Order.h"
#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
class AssetManager;
class PhysicsContext;

bagel::Entity createCustomer(PhysicsContext& physics, AssetManager& assets,
                           WorldPos pos, const Order order, float patience);
} // namespace cafe
