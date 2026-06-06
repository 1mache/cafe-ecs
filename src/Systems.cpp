#include "Systems.h"
#include "Components.h"
#include "Entities.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "RenderContext.h"
#include "TransformOperations.h"
#include <algorithm>
#include <bagel.h>
#include <box2d/box2d.h>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

namespace cafe
{
// Running counters for the periodic debug dump. File-local so tests don't see them.
namespace
{
struct DebugStats
{
    int   spawned{};
    int   caught{};
    int   spilled{};
    int   overflowed{};
    float accumulator{};
};
DebugStats g_stats;

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

    SDL_Window* window = RenderContext::getWindow();
    SDL_Renderer* renderer = RenderContext::getRenderer();
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        const auto& d = e.get<Drawable>();
        const auto& t = e.get<Transform>();

        SDL_FRect dstRect = transformToFrect(t, RenderContext::getCameraPos());

        if (e.has<Cup>())
            drawCupLiquid(renderer, dstRect, e.get<Cup>());

        SDL_RenderTexture(renderer, d.texture, &d.srcRect, &dstRect);
    }
}

void syncTransformFromBody()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Transform>().set<PhysicsBody>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;
        const auto body = e.get<PhysicsBody>().id;
        if (!b2Body_IsValid(body)) continue;  // belt + suspenders: never sync a destroyed body
        b2Vec2 p   = b2Body_GetPosition(body);
        b2Rot  rot = b2Body_GetRotation(body);
        auto&  t   = e.get<Transform>();
        t.x   = p.x;
        t.y   = p.y;
        t.rot = b2Rot_GetAngle(rot);
    }
}

void coffeeSpawnerSystemImpl(float dtSeconds,
                             const std::function<void(WorldPos)>& spawnDrop)
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<CoffeeSpawner>().set<Transform>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;
        auto& s = e.get<CoffeeSpawner>();
        if (!s.active) continue;

        const auto& t = e.get<Transform>();
        s.accumulator += dtSeconds;
        // while() so a long frame can fire multiple drops in one tick
        while (s.accumulator >= s.interval)
        {
            s.accumulator -= s.interval;
            spawnDrop({ t.x + s.offset.x, t.y + s.offset.y });
        }
    }
}

void coffeeSpawnerSystem(float dtSeconds)
{
    // We don't need to track each drop's entity; the sensor system finds them
    // again through their bodies' userData when destroying them.
    coffeeSpawnerSystemImpl(dtSeconds, [](WorldPos p) {
        (void)createLiquidDrop(p);
        ++g_stats.spawned;
    });
}

void sensorEventSystem()
{
    const b2SensorEvents events = b2World_GetSensorEvents(PhysicsContext::world());

    // A drop can show up in more than one begin-event per step (e.g. it enters
    // CUP_INSIDE while also brushing CLEANUP, or overlaps two cups). Collect
    // the drops to destroy, dedupe, then destroy once per id.
    std::vector<bagel::ent_type> toDestroy;
    toDestroy.reserve(static_cast<size_t>(events.beginCount));

    static const bagel::Mask liquidMask = bagel::MaskBuilder().set<Liquid>().build();
    static const bagel::Mask cupMask    = bagel::MaskBuilder().set<Cup>().build();

    for (int i = 0; i < events.beginCount; ++i)
    {
        const auto& be = events.beginEvents[i];

        // Skip events whose shapes were destroyed since the buffer was filled.
        // Box2D 3 versions shapes by generation; using a stale id trips an assert.
        if (!b2Shape_IsValid(be.visitorShapeId)) continue;
        if (!b2Shape_IsValid(be.sensorShapeId))  continue;

        // Recover the visitor (= the drop) entity from the body's userData.
        const b2BodyId visitorBody = b2Shape_GetBody(be.visitorShapeId);
        const bagel::ent_type visitorId{
            static_cast<int>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(visitorBody)))
        };
        bagel::Entity visitor{ visitorId };
        if (!visitor.test(liquidMask)) continue;

        // Which sensor was hit? Read the category bit on the sensor shape.
        const uint64_t sensorCat = b2Shape_GetFilter(be.sensorShapeId).categoryBits;

        if (sensorCat & filter::CUP_INSIDE)
        {
            const b2BodyId cupBody = b2Shape_GetBody(be.sensorShapeId);
            bagel::Entity cup{ bagel::ent_type{
                static_cast<int>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(cupBody)))
            } };

            // Cup full → deflect sideways + upward. CLEANUP destroys the drop later.
            if (cup.test(cupMask) && cup.get<Cup>().isFull())
            {
                const b2BodyId dropBody = b2Shape_GetBody(be.visitorShapeId);
                const b2Vec2   dropPos  = b2Body_GetPosition(dropBody);
                const b2Vec2   cupPos   = b2Body_GetPosition(cupBody);
                const float    side     = (dropPos.x >= cupPos.x) ? 1.f : -1.f;
                b2Body_SetLinearVelocity(dropBody, { side * 3.f, 5.f });

                if (g_stats.overflowed == 0)
                    std::cout << "[Spill] cup overflowed for the first time" << std::endl;
                ++g_stats.overflowed;
                continue;
            }

            if (cup.test(cupMask))
            {
                auto& c = cup.get<Cup>();
                ++c.filled;
                if (c.filled == c.capacity)
                    std::cout << "[CupFull] cup is full (" << c.filled
                              << "/" << c.capacity << ")" << std::endl;
            }
            ++g_stats.caught;
            toDestroy.push_back(visitorId);
        }
        else if (sensorCat & filter::CLEANUP)
        {
            ++g_stats.spilled;
            toDestroy.push_back(visitorId);
        }
    }

    std::sort(toDestroy.begin(), toDestroy.end(),
              [](bagel::ent_type a, bagel::ent_type b) { return a.id < b.id; });
    toDestroy.erase(
        std::unique(toDestroy.begin(), toDestroy.end(),
                    [](bagel::ent_type a, bagel::ent_type b) { return a.id == b.id; }),
        toDestroy.end());

    for (auto id : toDestroy)
        destroyPhysicalEntity(id);
}

void dumpDebugStatsEvery(float dtSeconds)
{
    constexpr float interval = 0.5f;
    g_stats.accumulator += dtSeconds;
    if (g_stats.accumulator < interval) return;
    g_stats.accumulator = 0.f;

    static const bagel::Mask cupMask    = bagel::MaskBuilder().set<Cup>().build();
    static const bagel::Mask liquidMask = bagel::MaskBuilder().set<Liquid>().build();

    int totalFilled = 0, totalCapacity = 0, liveDrops = 0;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(cupMask))
        {
            const auto& c = e.get<Cup>();
            totalFilled   += c.filled;
            totalCapacity += c.capacity;
        }
        if (e.test(liquidMask)) ++liveDrops;
    }

    const int pct = totalCapacity ? (totalFilled * 100) / totalCapacity : 0;
    std::cout << "[Stats] spawned=" << g_stats.spawned
              << " caught="     << g_stats.caught
              << " overflowed=" << g_stats.overflowed
              << " spilled="    << g_stats.spilled
              << " live="       << liveDrops
              << " cup="        << totalFilled << "/" << totalCapacity
              << " (" << pct << "%)" << std::endl;
}
} // namespace cafe
