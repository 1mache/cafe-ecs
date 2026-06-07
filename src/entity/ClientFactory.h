#pragma once

#include "Order.h"
#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
bagel::Entity createClient(SDL_Texture* tex, float texW, float texH,
                           WorldPos pos, const Order& order, float patience,
                           SDL_FPoint mouthOffsetPx);
} // namespace cafe
