#include "CheckBeverageSystem.h"
#include "Components.h"
#include "OrderMatch.h"
#include <vector>

namespace cafe
{
void acceptGradedBeverageSystem()
{
    struct Handoff { bagel::ent_type item; bagel::ent_type customer; };
    std::vector<Handoff> handoffs;

    static const bagel::Mask mask =
        bagel::MaskBuilder().set<CoffeeOverview>().set<CheckCoffeeIntent>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        const auto& intent = e.get<CheckCoffeeIntent>();
        bagel::Entity customer{ intent.customer };
        if (!customer.has<Order>() || !customer.has<Served>()) continue;

        auto& served = customer.get<Served>();
        served.drink      = true;
        served.drinkGrade = gradeDrink(intent, e.get<CoffeeOverview>());
        handoffs.push_back({ e.entity(), intent.customer });
        e.remove<CheckCoffeeIntent>();
    }

    for (const auto& h : handoffs)
        bagel::Entity{ h.item }.add(DeliveredTo{ h.customer });
}
} // namespace cafe
