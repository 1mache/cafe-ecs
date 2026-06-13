#include "RenderSystem.h"
#include "Components.h"
#include "RenderContext.h"
#include "Transform.h"
#include <algorithm>
#include <vector>
#include <SDL3/SDL.h>
#include <bagel.h>
#include <vector>
#include <cmath>

namespace cafe
{
namespace
{
// Per-ingredient color, shared by drop tinting.
SDL_Color ingredientColor(Ingredient kind)
{
    switch (kind)
    {
    case Ingredient::Coffee: return { 75,  47,  30,  255 }; // #4B2F1E
    case Ingredient::Milk:   return { 240, 234, 214, 255 }; // #F0EAD6
    case Ingredient::Water:  return { 111, 183, 224, 255 }; // #6FB7E0
    default:                 return { 255, 255, 255, 255 };
    }
}


} // namespace

void drawSystem(SDL_Renderer* renderer)
{
    using Entity = bagel::Entity;

    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Drawable>().set<Transform>().build();

    std::vector<Entity> drawables{};
    for (auto e = Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask))
            continue;

        drawables.emplace_back(e);
    }

    // TODO: replace dummy sorting with performant storage type
    auto layerLess = [](const Entity a, const Entity b)
    {
        const auto& da = a.get<Drawable>();
        const auto& db = b.get<Drawable>();

        return da.renderLayer < db.renderLayer;
    };

    // sort based on render layer
    std::ranges::sort(drawables, layerLess);

    for (auto e: drawables)
    {
        const auto& d = e.get<Drawable>();
        const auto& t = e.get<Transform>();

        SDL_FRect dstRect = transformToFrect(t, RenderContext::getCameraPos());

        // tint particles to their ingredient color (shared particle.png)
        if (e.has<Liquid>())
        {
            const SDL_Color col = ingredientColor(e.get<Liquid>().kind);
            SDL_SetTextureColorMod(d.texture, col.r, col.g, col.b);
        }

        SDL_RenderTexture(renderer, d.texture, &d.srcRect, &dstRect);

        if (e.has<Liquid>())
            SDL_SetTextureColorMod(d.texture, 255, 255, 255);
    }
}
} // namespace cafe
