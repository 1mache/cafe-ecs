#include "CustomerSystem.h"
#include "Components.h"
#include "Entities.h"
#include "Menu.h"
#include <bagel.h>
#include <box2d/box2d.h>
#include <cmath>
#include <iostream>
#include <vector>

namespace cafe
{
namespace
{
constexpr float REJECT_IMPULSE = 300.f; // N·s applied to a rejected item
constexpr float PI = 3.14159f;

void rejectItem(bagel::Entity e)
{
    if (!e.has<PhysicsBody>()) return;
    const b2BodyId body = e.get<PhysicsBody>().id;
    if (!b2Body_IsValid(body)) return;

    b2Body_SetGravityScale(body, 1.f);
    b2Vec2 incomingVelocity = b2Body_GetLinearVelocity(body);
    // zero out so by the end only our impulse has effect
    b2Body_SetLinearVelocity(body, {});
    // reject vector is the opposite to the incoming velocity vector
    float rejectAngle = std::atan2(-incomingVelocity.y, -incomingVelocity.x);
    b2Body_ApplyLinearImpulseToCenter(
        body, { REJECT_IMPULSE * std::cos(rejectAngle), REJECT_IMPULSE * std::sin(rejectAngle) }, true);
}
} // namespace

void customerSpawnerSystem(AssetManager& assets, PhysicsContext& physics, float dtSeconds)
{
    static const bagel::Mask spawnerMask =
        bagel::MaskBuilder().set<Spawner>().build();
    static const bagel::Mask customerMask =
        bagel::MaskBuilder().set<Order>().set<Behavior>().build();

    // The seat is "occupied" while any customer exists.
    int customers = 0;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
        if (e.test(customerMask)) ++customers;

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(spawnerMask)) continue;
        auto& sp = e.get<Spawner>();

        // Seat busy: hold the timer armed so the full interval is waited once it frees.
        if (customers > 0)
        {
            sp.cooldown = sp.interval;
            continue;
        }

        sp.cooldown -= dtSeconds;
        if (sp.cooldown <= 0.f)
        {
            spawnCustomer(assets, physics, sp.seat, randomOrder(), sp.patience);
            sp.cooldown = sp.interval;
        }
    }
}

void behaviorSystem(float dtSeconds)
{
    static const bagel::Mask behaviorMask =
        bagel::MaskBuilder().set<Behavior>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(behaviorMask)) continue;

        auto& behavior = e.get<Behavior>();
        behavior.patience -= dtSeconds;

        if (behavior.patience <= 0.f && !e.has<Leaving>())
            e.add(Leaving{});
    }
}

void deliverySystem()
{
    static const bagel::Mask dragMask =
        bagel::MaskBuilder().set<DragIntent>().build();

    // Pastries to destroy immediately after the loop (structural changes deferred).
    std::vector<bagel::ent_type> pastriesToDestroy;

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(dragMask)) continue;

        auto& intent = e.get<DragIntent>();
        if (intent.intentType != DragIntentType::released) continue;
        if (!intent.dropSpaceEntity.has_value()) continue;

        bagel::Entity target{ *intent.dropSpaceEntity };

        if (!target.has<Order>() || !target.has<OrderGrade>()) continue;

        const auto& order = target.get<Order>();
        auto& grade       = target.get<OrderGrade>();

        if (e.has<Cup>())
        {
            // Find the first unserved drink slot; grade via acceptGradedBeverageSystem.
            const int slot = firstUnservedDrink(order, grade);
            if (slot < 0 || e.has<CheckCoffeeIntent>())
            {
                rejectItem(e);
                intent.dropSpaceEntity = std::nullopt;
                continue;
            }

            const auto& recipe = recipeFor(order.drinks[slot].type);
            CheckCoffeeIntent coffeeIntent{};
            for (size_t i = 0; i < INGREDIENT_COUNT; ++i)
                coffeeIntent.ratio[i] = recipe.ratio[i];
            coffeeIntent.targetFill = recipe.targetFill;
            coffeeIntent.isHot    = order.drinks[slot].temp == Temperature::Hot;
            coffeeIntent.customer = target.entity();
            coffeeIntent.drinkSlot = slot;
            e.add(coffeeIntent);
        }
        else // pastry
        {
            const int slot = firstUnservedPastry(order, grade);
            if (slot < 0)
            {
                rejectItem(e);
                intent.dropSpaceEntity = std::nullopt;
                continue;
            }

            // Stub: pastry temperature grading not yet implemented; award full points.
            // Integration point for the microwave (see Microwave.h / microwaveSystem):
            // a heated pat carries Pastry.temperature == HEATED_TEMPERATURE. Grade it the
            // same way drinks compare isHot above, e.g.:
            //   const bool wantHot = order.pastries[slot].temp == Temperature::Hot;
            //   const bool isHot   = e.get<Pastry>().temperature == HEATED_TEMPERATURE;
            //   grade.pastryGrades[slot] = (wantHot == isHot) ? MAX_ITEM_GRADE : 0;
            grade.pastryGrades[slot] = MAX_ITEM_GRADE;
            markPastryServed(grade, slot);
            pastriesToDestroy.push_back(e.entity());
        }
    }

    // Destroy accepted pastries immediately (after iteration to avoid invalidating it).
    for (auto id : pastriesToDestroy)
        destroyDeliveredItem(id);
}

void orderSystem()
{
    static const bagel::Mask orderMask =
        bagel::MaskBuilder().set<Order>().set<Behavior>().set<OrderGrade>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(orderMask)) continue;
        if (e.has<Leaving>()) continue;

        if (allItemsServed(e.get<Order>(), e.get<OrderGrade>()))
            e.add(Leaving{});
    }
}

void finalizeOrderGradeSystem()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Leaving>().set<Behavior>().set<Order>().set<OrderGrade>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        auto& behavior       = e.get<Behavior>();
        const auto& order    = e.get<Order>();
        const auto& grade    = e.get<OrderGrade>();

        behavior.succeeded = allItemsServed(order, grade);

        const int raw = sumItemGrades(order, grade);

        // Patience penalty: two steps, capped at 25% reduction.
        float factor = 1.0f;
        if (behavior.patience <= 0.f)
            factor = 0.75f;
        else if (behavior.maxPatience > 0.f && behavior.patience <= 0.5f * behavior.maxPatience)
            factor = 0.875f;

        behavior.rating = static_cast<int>(std::round(static_cast<float>(raw) * factor));
    }
}

void reportLeavingCustomers()
{
    static const bagel::Mask leavingMask =
        bagel::MaskBuilder().set<Leaving>().set<Behavior>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(leavingMask)) continue;

        const auto& behavior = e.get<Behavior>();
        if (behavior.succeeded)
            std::cout << "[Order] Customer left SUCCESSFUL — rating: " << behavior.rating << "\n";
        else
            std::cout << "[Order] Customer left FAILED (patience ran out) — rating: " << behavior.rating << "\n";
    }
}

void customerCleanupSystem()
{
    static const bagel::Mask leavingMask =
        bagel::MaskBuilder().set<Leaving>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(leavingMask)) continue;
        destroyPhysicalEntity(e.entity());
    }
}

} // namespace cafe
