#include "Components.h"
#include "CustomerSystem.h"
#include "Entities.h"
#include "Menu.h"
#include "OrderMatch.h"
#include <bagel.h>
#include <box2d/box2d.h>
#include <iostream>
#include <vector>

namespace cafe
{
namespace
{
int gradeToRating(DrinkGrade g)
{
    switch (g)
    {
    case DrinkGrade::Perfect:    return 2;
    case DrinkGrade::Acceptable: return 1;
    default:                     return 0;
    }
}

// Sends a draggable item back to its supply slot: moves the physics body (so
// syncTransformFromBody doesn't overwrite it next frame) and stops it dead.
void recycleItem(bagel::Entity e)
{
    if (!e.has<HomeSlot>()) return;
    const WorldPos home = e.get<HomeSlot>().pos;

    auto& t = e.get<Transform>();
    t.x = home.x;
    t.y = home.y;

    if (e.has<PhysicsBody>())
    {
        const b2BodyId body = e.get<PhysicsBody>().id;
        if (b2Body_IsValid(body))
        {
            b2Body_SetTransform(body, { home.x, home.y }, b2Body_GetRotation(body));
            b2Body_SetLinearVelocity(body, { 0.f, 0.f });
        }
    }
}
} // namespace

void customerSpawnerSystem(float dtSeconds, AssetManager& assets)
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
            spawnCustomer(assets, sp.seat, randomDrinkOrder(/*hasPastry=*/ true), sp.patience);
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

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(dragMask)) continue;

        auto& intent = e.get<DragIntent>();
        if (intent.intentType != DragIntentType::released) continue;
        if (!intent.dropSpaceEntity.has_value()) continue;

        bagel::Entity target{ *intent.dropSpaceEntity };
        if (!target.has<Served>()) continue;

        auto& served = target.get<Served>();

        if (e.has<Cup>())
        {
            // Customer accepts any coffee; the grade reflects how well it matched.
            // The cup stays on the customer; it is recycled + emptied only once the
            // whole order is fulfilled (see recycleDeliveredItems).
            if (target.has<Order>())
            {
                served.drink      = true;
                served.drinkGrade = gradeDrinkRatio(target.get<Order>(), e.get<Cup>());
            }
        }
        else // pastry
        {
            if (target.has<Order>() && target.get<Order>().hasPastry)
            {
                served.pastry = true; // taken; recycled at order completion
            }
            else
            {
                // Not wanted: bounce straight back to its slot.
                recycleItem(e);
                intent.dropSpaceEntity = std::nullopt;
            }
        }
    }
}

void recycleDeliveredItems()
{
    static const bagel::Mask leavingCustomerMask =
        bagel::MaskBuilder().set<Leaving>().set<Served>().build();
    static const bagel::Mask itemMask =
        bagel::MaskBuilder().set<HomeSlot>().set<DragItemType>().build();
    static const bagel::Mask liquidMask =
        bagel::MaskBuilder().set<Liquid>().build();

    // What did the departing customer actually receive? Driven off Served so the
    // reset happens only when the order is done (success) or the customer gives up
    // (fail) — never mid-order. One customer at a time, so OR-ing is unambiguous.
    bool recycleDrink  = false;
    bool recyclePastry = false;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(leavingCustomerMask)) continue;
        const auto& served = e.get<Served>();
        recycleDrink  |= served.drink;
        recyclePastry |= served.pastry;
    }
    if (!recycleDrink && !recyclePastry) return;

    // Send the delivered cup/pastry back to their slots (single instance each).
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(itemMask)) continue;
        const DropType type = e.get<DragItemType>().dropType;

        if (recycleDrink && type == DropType::cup)
        {
            recycleItem(e);
            Cup& cup = e.get<Cup>();
            for (size_t i = 0; i < INGREDIENT_COUNT; ++i)
                cup.filled[i] = 0;
        }
        else if (recyclePastry && type == DropType::pastry)
        {
            recycleItem(e);
        }
    }

    // Cup emptied + home: now destroy its accumulated drops. Collect-then-destroy
    // so we never destroy entities mid-iteration. Guarantees zero live particles.
    if (recycleDrink)
    {
        std::vector<bagel::ent_type> drops;
        for (auto e = bagel::Entity::first(); !e.eof(); e.next())
            if (e.test(liquidMask))
                drops.push_back(e.entity());

        for (auto id : drops)
            destroyPhysicalEntity(id); // frees the b2Body too (leak-safe)
    }
}

void orderSystem()
{
    static const bagel::Mask orderMask =
        bagel::MaskBuilder().set<Order>().set<Behavior>().set<Served>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(orderMask)) continue;
        if (e.has<Leaving>()) continue;

        const auto& order  = e.get<Order>();
        const auto& served = e.get<Served>();

        const bool drinkOk  = !order.hasDrink  || served.drink;
        const bool pastryOk = !order.hasPastry || served.pastry;

        if (drinkOk && pastryOk)
        {
            // Drink quality drives the rating; pastry-only orders just succeed.
            e.get<Behavior>().rating =
                order.hasDrink ? gradeToRating(served.drinkGrade) : 1;
            e.add(Leaving{});
        }
    }
}

void reportLeavingCustomers()
{
    static const bagel::Mask leavingMask =
        bagel::MaskBuilder().set<Leaving>().set<Behavior>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(leavingMask)) continue;

        if (e.get<Behavior>().rating > 0)
            std::cout << "[Order] Client left SUCCESSFUL\n";
        else
            std::cout << "[Order] Client left FAILED (patience ran out)\n";
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
