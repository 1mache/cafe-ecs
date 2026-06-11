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
// Rim inset in screen pixels.
constexpr float kCupRimPx = 1.f;

// Per-ingredient color, shared by drop tinting and the cup fill bands.
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

// Drawn before the cup sprite so the sprite's rim masks the rect edges.
// Each ingredient is a colored band, stacked from the bottom up, so the cup
// visibly shows its mix.
void drawCupLiquid(SDL_Renderer* r, const SDL_FRect& cupRect, const Cup& c)
{
    const float interiorW = cupRect.w - 2.f * kCupRimPx;
    const float interiorH = cupRect.h - 2.f * kCupRimPx;
    if (interiorW <= 0.f || interiorH <= 0.f || c.capacity <= 0) return;

    const float x      = std::floor(cupRect.x + kCupRimPx);
    const float w      = std::floor(interiorW);
    const float bottom = cupRect.y + kCupRimPx + interiorH; // interior bottom edge
    float stacked = 0.f; // height already drawn up from the bottom

    for (size_t i = 0; i < INGREDIENT_COUNT; ++i)
    {
        if (c.filled[i] <= 0) continue;
        const float frac  = static_cast<float>(c.filled[i]) / static_cast<float>(c.capacity);
        const float bandH = std::floor(frac * interiorH);
        if (bandH <= 0.f) continue;

        const SDL_FRect band{ x, std::floor(bottom - stacked - bandH), w, bandH };
        const SDL_Color col = ingredientColor(static_cast<Ingredient>(i));
        SDL_SetRenderDrawColor(r, col.r, col.g, col.b, 255);
        SDL_RenderFillRect(r, &band);
        stacked += bandH;
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

        if (e.has<Cup>())
            drawCupLiquid(renderer, dstRect, e.get<Cup>());

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
