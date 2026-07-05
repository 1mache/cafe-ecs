#include "LiquidSystem.h"
#include "AssetManager.h"
#include "AudioContext.h"
#include "Components.h"
#include "Entities.h"
#include "PhysicsContext.h"
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
} // namespace cafe
