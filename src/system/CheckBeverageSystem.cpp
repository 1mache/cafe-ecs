#include "CheckBeverageSystem.h"
#include "Components.h"
#include "OrderMatch.h"
#include <vector>

namespace cafe
{
namespace
{
CoffeeOverview buildOverview(bagel::ent_type cupId)
{
    static const bagel::Mask liquidMask = bagel::MaskBuilder().set<Liquid>().build();

    int filled[INGREDIENT_COUNT]{};
    int dropSum = 0;

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(liquidMask)) continue;

        const auto& liquid = e.get<Liquid>();
        if (liquid.owner.id != cupId.id) continue;

        ++filled[static_cast<size_t>(liquid.kind)];
        ++dropSum;
    }

    CoffeeOverview overview{};
    overview.dropSum = dropSum;
    overview.isHot   = true;
    if (dropSum > 0)
    {
        const float total = static_cast<float>(dropSum);
        for (size_t i = 0; i < INGREDIENT_COUNT; ++i)
            overview.ratio[i] = static_cast<float>(filled[i]) / total;
    }
    return overview;
}
} // namespace

void checkBeverageSystem()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<CheckCoffeeIntent>().set<Cup>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        const CoffeeOverview overview = buildOverview(e.entity());
        if (e.has<CoffeeOverview>())
            e.get<CoffeeOverview>() = overview;
        else
            e.add(overview);
    }
}

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
