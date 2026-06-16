#include "Components.h"
#include "CustomerSystem.h"
#include "Entities.h"
#include "Menu.h"
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

void customerSpawnerSystem(PhysicsContext& physics, float dtSeconds, AssetManager& assets)
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
            spawnCustomer(physics, assets, sp.seat, randomOrder(), sp.patience);
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

    // (item, customer) pairs to tag once the loop ends (add<> is structural).
    struct Handoff { bagel::ent_type item; bagel::ent_type customer; };
    std::vector<Handoff> handoffs;

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(dragMask)) continue;

        auto& intent = e.get<DragIntent>();
        if (intent.intentType != DragIntentType::released) continue;
        if (!intent.dropSpaceEntity.has_value()) continue;

        bagel::Entity target{ *intent.dropSpaceEntity };

        auto& served = target.get<Served>();

        if (e.has<Cup>())
        {
            // Customer accepts any coffee; acceptGradedBeverageSystem grades once the
            // cup has CoffeeOverview. The item stays in the customer's tray and is
            // destroyed only when the order completes / the customer leaves (see clearDeliveredItems).
            if (target.has<Order>() && !e.has<CheckCoffeeIntent>())
            {
                const auto& order  = target.get<Order>();
                const auto& recipe = recipeFor(order.drinks[0].type);
                CheckCoffeeIntent coffeeIntent{};
                for (size_t i = 0; i < INGREDIENT_COUNT; ++i)
                    coffeeIntent.ratio[i] = recipe.ratio[i];
                coffeeIntent.isHot    = order.drinks[0].temp == Temperature::Hot;
                coffeeIntent.customer = target.entity();
                e.add(coffeeIntent);
            }
        }
    
        else // pastry
        {
            if (target.has<Order>() && target.get<Order>().hasPastry)
            {
                served.pastry = true; // taken; destroyed at order completion
                handoffs.push_back({ e.entity(), target.entity() });
            }
            else
            {
                // Not wanted: bounce straight back to its slot.
                recycleItem(e);
                intent.dropSpaceEntity = std::nullopt;
            }
        }
    }

    for (const auto& h : handoffs)
        bagel::Entity{ h.item }.add(DeliveredTo{ h.customer });
}

void clearDeliveredItems()
{
    static const bagel::Mask leavingMask   = bagel::MaskBuilder().set<Leaving>().build();
    static const bagel::Mask deliveredMask = bagel::MaskBuilder().set<DeliveredTo>().build();
    static const bagel::Mask liquidMask    = bagel::MaskBuilder().set<Liquid>().build();
    static const bagel::Mask childMask     = bagel::MaskBuilder().set<ChildOf>().build();

    // 1. Which customers are leaving this frame? Their whole tray gets wiped,
    //    whether the order succeeded or their patience ran out.
    std::vector<bagel::ent_type> leaving;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
        if (e.test(leavingMask)) leaving.push_back(e.entity());
    if (leaving.empty()) return;

    auto isLeaving = [&](int id) {
        for (auto c : leaving) if (c.id == id) return true;
        return false;
    };

    // 2. Items delivered to a leaving customer (multiple cups possible).
    std::vector<bagel::ent_type> items; // cups + pastries
    std::vector<bagel::ent_type> cups;  // subset, for drop/child cleanup
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(deliveredMask)) continue;
        if (!isLeaving(e.get<DeliveredTo>().customer.id)) continue;
        items.push_back(e.entity());
        if (e.has<Cup>()) cups.push_back(e.entity());
    }
    if (items.empty()) return;

    auto isDeliveredCup = [&](int id) {
        for (auto c : cups) if (c.id == id) return true;
        return false;
    };

    // 3. Each delivered cup's drops (by owner tag) and child sprite (cupFront).
    //    Scoped to these cups so other staged cups keep their contents.
    std::vector<bagel::ent_type> extra;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.test(liquidMask) && isDeliveredCup(e.get<Liquid>().owner.id))
            extra.push_back(e.entity());
        else if (e.test(childMask) && isDeliveredCup(e.get<ChildOf>().parent.entity().id))
            extra.push_back(e.entity());
    }

    // 4. Destroy everything (already collected — safe). Guarantees zero leftover
    //    particles/bodies and no orphaned cup-front sprites.
    for (auto id : items) destroyPhysicalEntity(id);
    for (auto id : extra) destroyPhysicalEntity(id);
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
