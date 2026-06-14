#pragma once

#include "Order.h"
#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
class AssetManager;

bagel::Entity createCustomer(AssetManager& assets,
                           WorldPos pos, const Order order, float patience);

/** @brief Creates a customer plus its speech bubble and order icons as one unit.
 *  Returns the customer entity (the bubble/icons are its children via ChildOf). */
bagel::Entity spawnCustomer(AssetManager& assets,
                            WorldPos pos, Order order, float patience);
} // namespace cafe
