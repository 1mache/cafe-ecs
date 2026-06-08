#include "RenderSystem.h"
#include "Components.h"
#include "RenderContext.h"
#include "Transform.h"
#include <SDL3/SDL.h>
#include <bagel.h>
#include <vector>
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

void drawSystem()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Drawable>().set<Transform>().build();

    struct DrawItem
    {
        bagel::ent_type id;
        int             renderLayer;
    };

    std::vector<DrawItem> drawOrder;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;
        drawOrder.push_back({ e.entity(), e.get<Drawable>().renderLayer });
    }

    std::sort(drawOrder.begin(), drawOrder.end(),
              [](const DrawItem& a, const DrawItem& b) { return a.renderLayer < b.renderLayer; });

    SDL_Renderer* renderer = RenderContext::getRenderer();
    for (const DrawItem& item : drawOrder)
    {
        bagel::Entity e{ item.id };
        const auto&   d = e.get<Drawable>();
        const auto&   t = e.get<Transform>();

        SDL_FRect dstRect = transformToFrect(t, RenderContext::getCameraPos());

        if (e.has<Cup>())
            drawCupLiquid(renderer, dstRect, e.get<Cup>());

        SDL_RenderTexture(renderer, d.texture, &d.srcRect, &dstRect);
    }
}
} // namespace cafe
