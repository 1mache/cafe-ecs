#pragma once
#include "SDL3/SDL_render.h"
#include "Texture.h"
#include "bagel.h"

namespace cafe
{
class PhysicsContext;
class AssetManager;

    // create bg image
    bagel::Entity createBg(AssetManager& assets, std::string_view bgPath);
    // create counter/bartop.
    bagel::Entity createBartop(AssetManager& assets, PhysicsContext& physicsContext);
}