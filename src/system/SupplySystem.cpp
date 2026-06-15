#include "SupplySystem.h"

#include "Components.h"
#include "Entities.h"
#include "Transform.h"
#include <bagel.h>
#include <box2d/box2d.h>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <vector>

namespace cafe
{
namespace
{
float distSq(float ax, float ay, float bx, float by)
{
    const float dx = ax - bx, dy = ay - by;
    return dx * dx + dy * dy;
}

// Cartoonish ease-out with a slight overshoot past the target before settling.
float easeOutBack(float p)
{
    constexpr float c1 = 1.70158f;
    constexpr float c3 = c1 + 1.f;
    const float q = p - 1.f;
    return 1.f + c3 * q * q * q + c1 * q * q;
}

// A slot is occupied if an item of this type is falling toward it or resting on it.
bool slotOccupied(WorldPos slot, DropType item)
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<DragItemType>().set<Transform>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;
        if (e.get<DragItemType>().dropType != item) continue;
        if (e.has<DeliveredTo>()) continue; // in a customer's tray, not staged supply

        const WorldPos at = e.has<Falling>()
                                ? e.get<Falling>().slot
                                : WorldPos{ e.get<Transform>().x, e.get<Transform>().y };
        if (distSq(at.x, at.y, slot.x, slot.y) < supply::SLOT_RADIUS * supply::SLOT_RADIUS)
            return true;
    }
    return false;
}

// Spawns the item at its slot, then lifts it above the screen and starts the fall.
void spawnFalling(PhysicsContext& physics, AssetManager& assets, DropType item, WorldPos slot)
{
    bagel::Entity e = (item == DropType::cup)
                          ? createCup(physics, assets, slot, supply::CUP_CAPACITY)
                          : createPastry(physics, slot, assets);

    e.get<Transform>().y = supply::DROP_FROM_Y;
    if (e.has<PhysicsBody>())
    {
        const b2BodyId body = e.get<PhysicsBody>().id;
        if (b2Body_IsValid(body))
        {
            b2Body_SetTransform(body, { slot.x, supply::DROP_FROM_Y }, b2Body_GetRotation(body));
            b2Body_SetLinearVelocity(body, { 0.f, 0.f });
        }
    }
    e.add(Falling{ .slot = slot, .fromY = supply::DROP_FROM_Y, .t = 0.f, .duration = supply::DROP_DURATION });
}
} // namespace

void supplyButtonSystem(PhysicsContext& physics, AssetManager& assets)
{
    static const bagel::Mask buttonMask = bagel::MaskBuilder().set<SpawnButton>().build();

    // Consume click flags during the scan (field mutation is safe); defer the
    // actual spawns out of the loop since spawning creates entities.
    std::vector<DropType> toSpawn;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(buttonMask)) continue;
        auto& b = e.get<SpawnButton>();
        if (!b.justPressed) continue;
        b.justPressed = false;
        toSpawn.push_back(b.item);
    }

    for (DropType item : toSpawn)
    {
        const WorldPos* slots = (item == DropType::cup) ? supply::CUP_SLOTS : supply::PASTRY_SLOTS;
        const size_t    count = (item == DropType::cup) ? std::size(supply::CUP_SLOTS)
                                                        : std::size(supply::PASTRY_SLOTS);
        for (size_t i = 0; i < count; ++i)
        {
            if (!slotOccupied(slots[i], item))
            {
                spawnFalling(physics, assets, item, slots[i]);
                break;
            }
        }
        // All slots full: ignore.
    }
}

void fallingSystem(float dtSeconds)
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Falling>().set<PhysicsBody>().build();

    std::vector<bagel::ent_type> landed;

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        auto&         f    = e.get<Falling>();
        const b2BodyId body = e.get<PhysicsBody>().id;
        if (!b2Body_IsValid(body)) continue;

        f.t += dtSeconds;
        const float p = (f.t >= f.duration) ? 1.f : f.t / f.duration;
        const float y = f.fromY + (f.slot.y - f.fromY) * easeOutBack(p);

        b2Body_SetTransform(body, { f.slot.x, y }, b2Body_GetRotation(body));
        b2Body_SetLinearVelocity(body, { 0.f, 0.f });

        if (f.t >= f.duration)
        {
            b2Body_SetTransform(body, { f.slot.x, f.slot.y }, b2Body_GetRotation(body));
            landed.push_back(e.entity());
        }
    }

    // Structural change deferred out of the loop: landed items become draggable.
    for (auto id : landed)
        bagel::Entity{ id }.del<Falling>();
}
} // namespace cafe
