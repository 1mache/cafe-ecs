#pragma once

#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
class AssetManager;

bagel::Entity createOrderIcon(AssetManager& assets, SDL_FRect srcRect,
                              float displayW, float displayH,
                              bagel::Entity parentBubble, SDL_FPoint offsetPx);
} // namespace cafe
