#include "RenderSystem.h"
#include "Components.h"
#include "RenderContext.h"
#include "Transform.h"
#include <algorithm>
#include <vector>
#include <SDL3/SDL.h>
#include <bagel.h>
#include <cmath>

namespace cafe
{
namespace
{
// Coffee fill color (#4B2F1E) and rim inset in screen pixels.
constexpr Uint8 kCoffeeR = 75, kCoffeeG = 47, kCoffeeB = 30;
constexpr float kCupRimPx = 1.f;

// Drawn before the cup sprite so the sprite's rim masks the rect edges.
void drawCupLiquid(SDL_Renderer* r, const SDL_FRect& cupRect, const Cup& c)
{
    const float interiorW = cupRect.w - 2.f * kCupRimPx;
    const float interiorH = cupRect.h - 2.f * kCupRimPx;
    if (interiorW <= 0.f || interiorH <= 0.f) return;

    const float fillH = std::floor(c.fillPercent() * interiorH);
    if (fillH <= 0.f) return;

    const SDL_FRect fillRect{
        std::floor(cupRect.x + kCupRimPx),
        std::floor(cupRect.y + kCupRimPx + (interiorH - fillH)),
        std::floor(interiorW),
        fillH
    };
    SDL_SetRenderDrawColor(r, kCoffeeR, kCoffeeG, kCoffeeB, 255);
    SDL_RenderFillRect(r, &fillRect);
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

        SDL_RenderTexture(renderer, d.texture, &d.srcRect, &dstRect);
    }
}
} // namespace cafe
