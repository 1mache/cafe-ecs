#include "RenderSystem.h"
#include "Components.h"
#include "PhysicsContext.h"
#include "RenderContext.h"
#include "Transform.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <bagel.h>
#include <box2d/box2d.h>
#include <cmath>
#include <iostream>
#include <ostream>
#include <vector>

namespace cafe
{
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

        // TODO: remove after sprite is added
        // Placeholder ice machine: a solid gray square (no art asset yet).
        if (e.has<IceMachine>())
        {
            SDL_SetRenderDrawColor(renderer, 150, 150, 160, 255);
            SDL_RenderFillRect(renderer, &dstRect);
            continue;
        }

        const SDL_Color& tint = d.tint;
        const bool hasTint = tint.r != 255 || tint.g != 255 || tint.b != 255 || tint.a != 255;
        if (hasTint)
        {
            SDL_SetTextureColorMod(d.texture, tint.r, tint.g, tint.b);
            SDL_SetTextureAlphaMod(d.texture, tint.a);
            SDL_SetTextureBlendMode(d.texture, tint.a < 255 ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_NONE);
        }

        SDL_RenderTexture(renderer, d.texture, &d.srcRect, &dstRect);

        if (hasTint)
        {
            SDL_SetTextureColorMod(d.texture, 255, 255, 255);
            SDL_SetTextureAlphaMod(d.texture, 255);
            SDL_SetTextureBlendMode(d.texture, SDL_BLENDMODE_NONE);
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
