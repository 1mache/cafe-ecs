#include "LiquidSystem.h"
#include "AssetManager.h"
#include "AudioContext.h"
#include "Components.h"
#include "Entities.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "SoundAssets.h"
#include "Utils.h"

#include <bagel.h>
#include <box2d/box2d.h>
#include <cstdint>

namespace cafe
{

    constexpr float COFFEE_POUR_SOUND_DELAY = 2.2f;
    constexpr float WATER_DROP_SOUND_DELAY = 0.15f;
void liquidSpawnerSystem(AssetManager& assets, PhysicsContext& physics, AudioContext& audio, float dtSeconds)
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<LiquidSpawner>().set<Transform>().build();

    static std::uniform_real_distribution jitter(-0.025f, 0.025f);

    bool coffeePouring = false;
    bool milkPouring   = false;
    bool waterPouring   = false;

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;
        auto& s = e.get<LiquidSpawner>();
        if (!s.active) continue;

        if (s.kind == LiquidIngredient::Coffee) coffeePouring = true;
        if (s.kind == LiquidIngredient::Milk)   milkPouring   = true;
        if (s.kind == LiquidIngredient::Water)  waterPouring   = true;

        const auto& t = e.get<Transform>();
        s.accumulator += dtSeconds;
        while (s.accumulator >= s.interval)
        {
            s.accumulator -= s.interval;
            (void)createLiquidDrop(assets, physics, { t.x + s.offset.x + jitter(getRng()), t.y + s.offset.y }, s.kind);
        }
    }

    // One sustained channel drives the pour sound. Coffee wins if both pour at once;
    // it stops the moment neither pipe is active.
    if (coffeePouring)
        audio.startSustained(sound::COFFEE, COFFEE_POUR_SOUND_DELAY, sound::POUR_VOLUME);
    else if (milkPouring)
        audio.startSustained(sound::MILK_STEAMER, 0.f, sound::POUR_VOLUME);
    else if (waterPouring)
        audio.startSustained(sound::WATER_DROP, WATER_DROP_SOUND_DELAY, sound::POUR_VOLUME);
    else
        audio.stopSustained();
}
void liquidVelocityClampSystem()
{
    constexpr float MAX_LIQUID_SPEED = 10.f;
    static bagel::Mask mask = bagel::MaskBuilder().set<Liquid>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        PhysicsBody& p = e.get<PhysicsBody>();
        auto vel = b2Body_GetLinearVelocity(p.id);
        if (b2Length(vel) > MAX_LIQUID_SPEED)
        {
            b2Body_SetLinearVelocity(p.id,b2Normalize(vel) * MAX_LIQUID_SPEED);
        }
    }
}

void liquidSensorEventSystem(PhysicsContext& physics)
{
    // Read the accumulated begin events: Box2D clears its own buffer each sub-step,
    // so PhysicsContext gathers every sub-step's events across the frame.
    const auto beginEvents = physics.sensorBeginEvents();

    static const bagel::Mask liquidMask = bagel::MaskBuilder().set<Liquid>().build();
    static const bagel::Mask cupMask    = bagel::MaskBuilder().set<Cup>().build();
    static const bagel::Mask iceMask    = bagel::MaskBuilder().set<Ice>().build();

    for (const auto& be : beginEvents)
    {

        // Skip events whose shapes were destroyed since the buffer was filled.
        if (!b2Shape_IsValid(be.visitorShapeId)) continue;
        if (!b2Shape_IsValid(be.sensorShapeId))  continue;

        // Recover the visitor (= the drop) entity from the body's userData.
        const b2BodyId visitorBody = b2Shape_GetBody(be.visitorShapeId);
        const bagel::ent_type visitorId{
            static_cast<int>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(visitorBody)))
        };
        bagel::Entity visitor{ visitorId };
        const bool visitorIsLiquid = visitor.test(liquidMask);
        const bool visitorIsIce    = visitor.test(iceMask);
        if (!visitorIsLiquid && !visitorIsIce) continue;

        // Which sensor was hit? Read the category bit on the sensor shape.
        const uint64_t sensorCat = b2Shape_GetFilter(be.sensorShapeId).categoryBits;

        if (sensorCat & filter::CUP_INSIDE)
        {
            const b2BodyId cupBody = b2Shape_GetBody(be.sensorShapeId);
            bagel::Entity cup{ bagel::ent_type{
                static_cast<int>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(cupBody)))
            } };

            if (!cup.test(cupMask))
                fatalError("Object with filter CUP_INSIDE detected that is not cup");

            auto& c = cup.get<Cup>();
            if (visitorIsIce)
            {
                // Count an ice cube once, the first time it enters any cup.
                auto& ice = visitor.get<Ice>();
                if (ice.holdingContainer.id < 0)
                {
                    ++c.iceCount;
                    ice.holdingContainer = cup.entity(); // tag for per-cup cleanup on delivery
                }
            }
            else // liquid drop
            {
                ++c.filled[static_cast<size_t>(visitor.get<Liquid>().kind)];
                visitor.get<Liquid>().holdingContainer = cup.entity(); // tag for per-cup cleanup on delivery
            }
        }
        else if (sensorCat & filter::CLEANUP)
        {
            // A drop can show up in more than one begin-event this frame (e.g.
            // brushing CLEANUP twice); Destroy is a tag, so re-tagging is a
            // no-op. destroySystem (end of frame) does the actual destroy + logs it.
            visitor.addAll(Destroy{});
        }
    }
}
} // namespace cafe
