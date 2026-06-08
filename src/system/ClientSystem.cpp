#include "ClientSystem.h"
#include "Components.h"
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

void orderSystem()
{
    static const bagel::Mask orderMask =
        bagel::MaskBuilder().set<Order>().set<Behavior>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(orderMask)) continue;

        // TODO: when a cup is served to this client, compare cup's Holds composition
        //       against Order::ratio and write the result into Behavior::rating
    }
}

void cleanupSystem()
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
