#include "Components.h"
#include "CustomerSystem.h"
#include "Entities.h"
#include "Menu.h"
#include "OrderMatch.h"
#include <bagel.h>
#include <iostream>

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

        const auto& intent = e.get<DragIntent>();
        if (intent.intentType != DragIntentType::released) continue;
        if (!intent.dropSpaceEntity.has_value()) continue;

        bagel::Entity target{ *intent.dropSpaceEntity };
        if (!target.has<Served>()) continue;

        auto& served = target.get<Served>();
        if (e.has<Cup>())
        {
            const Cup& cup = e.get<Cup>();
            // Only a sufficiently full cup counts as a served drink; its ratio
            // sets the grade.
            if (cup.fillPercent() >= MIN_SERVE_FILL && target.has<Order>())
            {
                served.drink      = true;
                served.drinkGrade = gradeDrinkRatio(target.get<Order>(), cup);
            }
        }
        else
        {
            served.pastry = true;
        }
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
