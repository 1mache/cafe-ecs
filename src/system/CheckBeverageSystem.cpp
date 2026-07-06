#include "CheckBeverageSystem.h"
#include "Components.h"
#include "CustomerSystem.h"
#include "Entities.h"
#include "Menu.h"
#include "OrderMatch.h"
#include <cmath>
#include <iostream>

namespace cafe
{
namespace
{
static constexpr const char* kIngredientNames[] = { "Coffee", "Water", "Milk" };

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

        const Cup& cup = e.get<Cup>();

        //**start score log
        std::cout << "[BeverageScore] snapshot (cup waiting on customer)\n";
        std::cout << "[BeverageScore]   actual drops: " << overview.dropSum
                  << " (" << static_cast<int>(overview.fillPercent * 100.f) << "% fill)\n";
        for (size_t i = 0; i < INGREDIENT_COUNT; ++i)
        {
            std::cout << "[BeverageScore]   actual " << kIngredientNames[i]
                      << " ratio: " << overview.ratio[i] << "\n";
        }
        std::cout << "[BeverageScore]   ice count: " << cup.iceCount
                  << ", ice found: " << (cup.iceCount > 0 ? "yes" : "no") << "\n";
        //**end score log
    }
}

void acceptGradedBeverageSystem()
{
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
            e.addAll(Destroy{}); // destroySystem cascades to contents/children
            continue;
        }

        const CoffeeOverview& ov    = e.get<CoffeeOverview>();
        const Order&          order = customer.get<Order>();
        auto&                 grade = customer.get<OrderGrade>();

        if (ov.dropSum == 0)
        {
            if (e.has<DragIntent>())
                e.get<DragIntent>().dropSpaceEntity = std::nullopt;
            e.del<CheckCoffeeIntent>();
            rejectItem(e);
            makeCustomerMad(customer);
            continue;
        }

        const int slot = matchDrinkSlotByRatio(order, grade, ov);

        if (slot < 0)
        {
            e.del<CheckCoffeeIntent>();
            e.addAll(Destroy{});
            makeCustomerMad(customer);
            continue;
        }

        const DrinkRecipe& recipe     = recipeFor(order.drinks[slot].type);
        const bool         expectedHot = order.drinks[slot].temp == Temperature::Hot;
        const Cup&         cup        = e.get<Cup>();
        const int expectedDrops = static_cast<int>(
            std::lround(recipe.targetFill * static_cast<float>(cup.capacity)));

        //**start score log
        std::cout << "[BeverageScore] === Beverage submitted ===\n";
        std::cout << "[BeverageScore] drink slot: " << slot
                  << ", drink: " << recipe.name << "\n";
        for (size_t i = 0; i < INGREDIENT_COUNT; ++i)
        {
            std::cout << "[BeverageScore]   expected " << kIngredientNames[i]
                      << " ratio: " << recipe.ratio[i] << "\n";
        }
        std::cout << "[BeverageScore]   expected drops: " << expectedDrops
                  << " (targetFill " << recipe.targetFill
                  << " * capacity " << cup.capacity << ")\n";
        std::cout << "[BeverageScore]   expected temperature: "
                  << (expectedHot ? "Hot" : "Cold") << "\n";
        std::cout << "[BeverageScore]   actual drops: " << ov.dropSum
                  << " (" << static_cast<int>(ov.fillPercent * 100.f) << "% fill)\n";
        for (size_t i = 0; i < INGREDIENT_COUNT; ++i)
        {
            std::cout << "[BeverageScore]   actual " << kIngredientNames[i]
                      << " ratio: " << ov.ratio[i] << "\n";
        }
        std::cout << "[BeverageScore]   ice count: " << cup.iceCount
                  << ", ice found: " << (cup.iceCount > 0 ? "yes" : "no") << "\n";
        std::cout << "[BeverageScore]   actual temperature: "
                  << (ov.isHot ? "Hot" : "Cold") << "\n";
        //**end score log

        grade.drinkGrades[slot] = gradeDrink(recipe, expectedHot, ov);

        //**start score log
        std::cout << "[BeverageScore] final drinkGrades[" << slot << "] = "
                  << grade.drinkGrades[slot] << "\n";
        //**end score log

        markDrinkServed(grade, slot);
        calmCustomer(customer);

        e.del<CheckCoffeeIntent>();
        e.addAll(Destroy{});
    }
}
} // namespace cafe
