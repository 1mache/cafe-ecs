#pragma once
#include "SDL3/SDL_render.h"
#include "Texture.h"
#include "bagel.h"

namespace cafe
{
    class AssetManager;

    bagel::Entity createBg(const Texture& bgTex);
    bagel::Entity createBartop(AssetManager& assets);
}