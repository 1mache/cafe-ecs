#include "Components.h"
#include "CustomerSystem.h"
#include <bagel.h>
#include <iostream>

namespace cafe
{

void behaviorSystem(float dtSeconds)
{
    static const bagel::Mask behaviorMask =
        bagel::MaskBuilder().set<Behavior>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(behaviorMask)) continue;

        auto& behavior = e.get<Behavior>();
        behavior.patience -= dtSeconds;

        // DEBUG: print patience once per second
        static float printAccum = 0.f;
        printAccum += dtSeconds;
        if (printAccum >= 1.f)
        {
            std::cout << "[behaviorSystem] entity " << e.entity().id
                      << " patience: " << behavior.patience << "s\n";
            printAccum = 0.f;
        }

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
            served.drink = served.drink || e.get<Cup>().isFull();
        else
            served.pastry = true;
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
            e.get<Behavior>().rating = 1;
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
        e.destroy();
    }
}

} // namespace cafe
