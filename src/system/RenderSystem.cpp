#include "RenderSystem.h"
#include "Components.h"
#include "Glyph.h" // alignedX
#include "PhysicsContext.h"
#include "RenderContext.h"
#include "Text.h"  // drawText
#include "Transform.h"
#include <SDL3/SDL.h>
#include <bagel.h>
#include <box2d/box2d.h>
#include <algorithm>
#include <cmath>

namespace cafe
{
namespace
{
using Entity = bagel::Entity;

void removeFromSortedDrawables(
    bagel::Bag<Entity, bagel::InitialEntities>& sorted,
    Entity e)
{
    const bagel::id_type id = e.entity().id;
    for (int i = 0; i < sorted.size(); ++i)
    {
        if (sorted[i].entity().id != id)
            continue;

        sorted[i] = sorted[sorted.size() - 1];
        sorted.pop();
        return;
    }
}

void sortDrawablesIfDirty(
    bagel::Bag<Entity, bagel::InitialEntities>& sorted,
    bool& dirtyBit)
{
    if (!dirtyBit)
        return;

    // Full sort every dirty frame. A single bubble pass could not keep up with
    // heavy add/remove churn (e.g. fast pouring spawns many LIQUID drops per
    // frame while removeFromSortedDrawables' swap-with-last breaks the order),
    // leaving high-layer icons mis-ordered for several frames -> visible flicker.
    // stable_sort keeps intra-layer order steady so same-layer sprites don't swap.
    if (sorted.size() > 1)
    {
        std::stable_sort(
            &sorted[0], &sorted[0] + sorted.size(),
            [](const Entity& a, const Entity& b)
            { return a.get<Drawable>().renderLayer < b.get<Drawable>().renderLayer; });
    }

    dirtyBit = false;
}
} // namespace

void drawSystem(SDL_Renderer* renderer)
{
    static bagel::Bag<Entity, bagel::InitialEntities> sortedDrawables{};
    static bagel::Bag<bool, bagel::InitialEntities>   inSortedDrawables{};
    static bool                                       dirtyBit = false;
    // bagel::DynamicBag is malloc-backed and never zero-initialized,
    // so a never-written slot   holds whatever garbage was in that heap byte (0xCD in
    // MSVC debug builds) instead of false. Explicitly zero each slot the first time
    // its index is reached before ever reading it.
    static bagel::id_type initializedUpTo = 0;

    for (auto e = Entity::first(); !e.eof(); e.next())
    {
        inSortedDrawables.ensure(e.entity().id + 1);
        while (initializedUpTo <= e.entity().id)
        {
            inSortedDrawables[initializedUpTo] = false;
            ++initializedUpTo;
        }
        const bool hasDrawable = e.has<Drawable>();
        const bool inSorted    = inSortedDrawables[e.entity().id];

        if (hasDrawable && !inSorted)
        {
            sortedDrawables.push(e);
            inSortedDrawables[e.entity().id] = true;
            dirtyBit = true;
        }
        else if (!hasDrawable && inSorted)
        {
            removeFromSortedDrawables(sortedDrawables, e);
            inSortedDrawables[e.entity().id] = false;
            dirtyBit = true;
        }
    }

    sortDrawablesIfDirty(sortedDrawables, dirtyBit);

    for (int i = 0; i < sortedDrawables.size(); ++i)
    {
        const Entity e = sortedDrawables[i];
        const auto& d = e.get<Drawable>();
        const auto& t = e.get<Transform>();

        SDL_FRect dstRect = transformToFrect(t, RenderContext::getCameraPos());

        // Solid colour quad: a Drawable with no texture fills its rect with tint.
        // Generalizes the placeholder above; used by flat UI like shop buttons.
        if (d.texture == nullptr)
        {
            const SDL_Color& c = d.tint;
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
            SDL_RenderFillRect(renderer, &dstRect);
            continue;
        }

        const SDL_Color& tint = d.tint;
        const bool hasTint = tint.r != 255 || tint.g != 255 || tint.b != 255 || tint.a != 255;

        // Sprites carry an alpha channel, so always blend. Using NONE leaves
        // transparent pixels opaque-black, and the mode is sticky per-texture —
        // shared textures (cup.png, particle.png) would then render black on the
        // next untinted draw.
        SDL_SetTextureBlendMode(d.texture, SDL_BLENDMODE_BLEND);
        if (hasTint)
        {
            SDL_SetTextureColorMod(d.texture, tint.r, tint.g, tint.b);
            SDL_SetTextureAlphaMod(d.texture, tint.a);
        }

        SDL_RenderTexture(renderer, d.texture, &d.srcRect, &dstRect);

        // Reset the (sticky, per-texture) mods so a shared texture isn't left tinted.
        if (hasTint)
        {
            SDL_SetTextureColorMod(d.texture, 255, 255, 255);
            SDL_SetTextureAlphaMod(d.texture, 255);
        }
    } 
}
void drawTextSystem(SDL_Renderer* renderer, const Texture& font)
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<TextLabel>().set<Transform>().build();

    const WorldPos cam = RenderContext::getCameraPos();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        const auto& label = e.get<TextLabel>();
        const auto& t     = e.get<Transform>();

        const SDL_FPoint anchor = worldToScreenPoint({ t.x, t.y }, cam);
        const float x = alignedX(label.text, label.scale, label.align, anchor.x);

        const SDL_Color& tint = label.tint;
        const bool hasTint = tint.r != 255 || tint.g != 255 || tint.b != 255 || tint.a != 255;
        if (hasTint)
        {
            SDL_SetTextureColorMod(font.get(), tint.r, tint.g, tint.b);
            SDL_SetTextureAlphaMod(font.get(), tint.a);
        }

        drawText(renderer, font, label.text, x, anchor.y, label.scale);

        if (hasTint)
        {
            SDL_SetTextureColorMod(font.get(), 255, 255, 255);
            SDL_SetTextureAlphaMod(font.get(), 255);
        }
    }
}
void debugHighlightPhysics(SDL_Renderer* renderer)
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<PhysicsBody>().build();

    const WorldPos cam = RenderContext::getCameraPos();
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 20, 147, 100); // hot pink, half transparent

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        const b2BodyId body = e.get<PhysicsBody>().id;
        const b2Transform xf = b2Body_GetTransform(body);

        b2ShapeId shapes[16];
        const int count = b2Body_GetShapes(body, shapes, 16);
        for (int i = 0; i < count; ++i)
        {
            if (b2Shape_GetType(shapes[i]) != b2_polygonShape) continue;
            if (b2Shape_IsSensor(shapes[i])) continue;

            const b2Polygon poly = b2Shape_GetPolygon(shapes[i]);
            for (int v = 0; v < poly.count; ++v)
            {
                const b2Vec2 wA = b2TransformPoint(xf, poly.vertices[v]);
                const b2Vec2 wB = b2TransformPoint(xf, poly.vertices[(v + 1) % poly.count]);

                const SDL_FPoint sA = worldToScreenPoint({ wA.x, wA.y }, cam);
                const SDL_FPoint sB = worldToScreenPoint({ wB.x, wB.y }, cam);
                SDL_RenderLine(renderer, sA.x, sA.y, sB.x, sB.y);
            }
        }
    }

    // Sensors — neon green
    SDL_SetRenderDrawColor(renderer, 57, 255, 20, 160);
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        const b2BodyId body = e.get<PhysicsBody>().id;
        const b2Transform xf = b2Body_GetTransform(body);

        b2ShapeId shapes[16];
        const int count = b2Body_GetShapes(body, shapes, 16);
        for (int i = 0; i < count; ++i)
        {
            if (b2Shape_GetType(shapes[i]) != b2_polygonShape) continue;
            if (!b2Shape_IsSensor(shapes[i])) continue;

            const b2Polygon poly = b2Shape_GetPolygon(shapes[i]);
            for (int v = 0; v < poly.count; ++v)
            {
                const b2Vec2 wA = b2TransformPoint(xf, poly.vertices[v]);
                const b2Vec2 wB = b2TransformPoint(xf, poly.vertices[(v + 1) % poly.count]);

                const SDL_FPoint sA = worldToScreenPoint({ wA.x, wA.y }, cam);
                const SDL_FPoint sB = worldToScreenPoint({ wB.x, wB.y }, cam);
                SDL_RenderLine(renderer, sA.x, sA.y, sB.x, sB.y);
            }
        }
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}
} // namespace cafe
