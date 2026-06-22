#include "CheckBeverageSystem.h"
#include "Components.h"
#include "Entities.h"
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
        if (liquid.holdingContainer.id != cupId.id) continue;

        ++filled[static_cast<size_t>(liquid.kind)];
        ++dropSum;
    }

    bagel::Entity cup{ cupId };
    const Cup& cupData  = cup.get<Cup>();
    const int iceCount  = cupData.iceCount;

    CoffeeOverview overview{};
    overview.dropSum    = dropSum;
    overview.fillPercent = cupData.capacity
        ? static_cast<float>(dropSum) / static_cast<float>(cupData.capacity)
        : 0.f;
    overview.isHot      = (iceCount == 0); // any ice => Cold
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
    std::vector<bagel::ent_type> toDestroy;

    static const bagel::Mask mask =
        bagel::MaskBuilder().set<CoffeeOverview>().set<CheckCoffeeIntent>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        const auto& intent = e.get<CheckCoffeeIntent>();
        bagel::Entity customer{ intent.customer };

        // Guard: customer may have left already — free the cup's intent and destroy it.
        if (!customer.has<Order>() || !customer.has<OrderGrade>())
        {
            e.del<CheckCoffeeIntent>();
            toDestroy.push_back(e.entity());
            continue;
        }

        const int slot  = intent.drinkSlot;
        auto& grade     = customer.get<OrderGrade>();
        grade.drinkGrades[slot] = gradeDrink(intent, e.get<CoffeeOverview>());
        markDrinkServed(grade, slot);

        e.del<CheckCoffeeIntent>();
        toDestroy.push_back(e.entity());
    }

    // Destroy cups (and their contents) after the iteration is complete.
    for (auto id : toDestroy)
        destroyDeliveredItem(id);
}
} // namespace cafe
