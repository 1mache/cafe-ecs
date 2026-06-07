#pragma once

#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
bagel::Entity createOrderIcon(SDL_Texture* tex, SDL_FRect srcRect,
                              float displayW, float displayH,
                              bagel::Entity parentBubble, SDL_FPoint offsetPx);
} // namespace cafe
