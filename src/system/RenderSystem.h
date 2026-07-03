#pragma once
#include "RenderLayers.h"
#include "SDL3/SDL_render.h"
#include "Texture.h"

namespace cafe
{
void drawSystem(SDL_Renderer* renderer);
void drawTextSystem(SDL_Renderer* renderer, const Texture& font);
void debugHighlightPhysics(SDL_Renderer* renderer);
} // namespace cafe
