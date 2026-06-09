#pragma once

#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
class AssetManager;

bagel::Entity createSpeechBubble(AssetManager& assets,
                                 float displayW, float displayH,
                                 bagel::Entity parent, SDL_FPoint offsetPx);
} // namespace cafe
