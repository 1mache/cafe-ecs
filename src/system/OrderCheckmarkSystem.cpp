#include "OrderCheckmarkSystem.h"
#include "Components.h"
#include <bagel.h>

namespace cafe
{
void orderCheckmarkSystem()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<Checkmark>().set<ChildOf>().set<Drawable>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        // Walk up the hierarchy: checkmark -> bubble -> customer.
        bagel::Entity bubble = e.get<ChildOf>().parent;
        if (!bubble.has<ChildOf>()) continue;
        bagel::Entity customer = bubble.get<ChildOf>().parent;
        if (!customer.has<OrderGrade>()) continue;

        const Checkmark&  c = e.get<Checkmark>();
        const OrderGrade& g = customer.get<OrderGrade>();
        const bool served = c.isDrink ? isDrinkServed(g, c.slot)
                                      : isPastryServed(g, c.slot);
        if (!served) continue;

        e.get<Drawable>().tint.a = 255; // reveal the checkmark
        e.del<Checkmark>();             // served: stop polling this one
    }
}
} // namespace cafe