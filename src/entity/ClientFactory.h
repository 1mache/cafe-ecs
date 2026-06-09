#pragma once

#include "Order.h"
#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
class AssetManager;

bagel::Entity createClient(AssetManager& assets,
                           WorldPos pos, const Order& order, float patience,
                           SDL_FPoint mouthOffsetPx);
} // namespace cafe
