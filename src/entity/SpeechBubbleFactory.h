#pragma once

#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
bagel::Entity createSpeechBubble(SDL_Texture* tex, SDL_FRect srcRect,
                                 float displayW, float displayH,
                                 bagel::Entity parent, SDL_FPoint offsetPx);
} // namespace cafe
