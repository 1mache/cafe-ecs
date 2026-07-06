#include "OrderGrade.h"
#include "Menu.h"
#include <cmath>

namespace cafe
{
namespace
{
static float drinkRatioDelta(const Order& order, int slot, const CoffeeOverview& overview)
{
    const DrinkRecipe& recipe = recipeFor(order.drinks[slot].type);
    float              delta  = 0.f;
    for (size_t j = 0; j < INGREDIENT_COUNT; ++j)
        delta += std::fabs(recipe.ratio[j] - overview.ratio[j]);
    return delta;
}
} // namespace

int firstUnservedDrink(const Order& order, const OrderGrade& grade)
{
    for (int i = 0; i < order.drinkCount; ++i)
        if (!isDrinkServed(grade, i)) return i;
    return -1;
}

int firstUnservedPastry(const Order& order, const OrderGrade& grade)
{
    for (int j = 0; j < order.pastryCount; ++j)
        if (!isPastryServed(grade, j)) return j;
    return -1;
}

bool allItemsServed(const Order& order, const OrderGrade& grade)
{
    for (int i = 0; i < order.drinkCount; ++i)
        if (!isDrinkServed(grade, i)) return false;
    for (int j = 0; j < order.pastryCount; ++j)
        if (!isPastryServed(grade, j)) return false;
    return true;
}

int sumItemGrades(const Order& order, const OrderGrade& grade)
{
    int total = 0;
    for (int i = 0; i < order.drinkCount; ++i)
        total += grade.drinkGrades[i];
    for (int j = 0; j < order.pastryCount; ++j)
        total += grade.pastryGrades[j];
    return total;
}

int matchDrinkSlotByRatio(const Order& order, const OrderGrade& grade, const CoffeeOverview& overview)
{
    int   bestSlot  = -1;
    float bestDelta = 0.f;

    for (int i = 0; i < order.drinkCount; ++i)
    {
        if (isDrinkServed(grade, i)) continue;

        const float delta = drinkRatioDelta(order, i, overview);
        if (bestSlot < 0 || delta < bestDelta)
        {
            bestSlot  = i;
            bestDelta = delta;
        }
    }

    if (bestSlot < 0) return -1;

    for (int i = 0; i < order.drinkCount; ++i)
    {
        if (isDrinkServed(grade, i)) continue;
        if (drinkRatioDelta(order, i, overview) != bestDelta) continue;

        const bool tempMatches = (order.drinks[i].temp == Temperature::Hot) == overview.isHot;
        if (tempMatches) return i;
    }

    return bestSlot;
}

int matchPastrySlot(const Order& order, const OrderGrade& grade,
                    PastryType pastryType, Temperature pastryTemp)
{
    int lastTypeMatch = -1;
    for (int j = 0; j < order.pastryCount; ++j)
    {
        if (isPastryServed(grade, j)) continue;
        if (order.pastries[j].type != pastryType) continue;
        if (order.pastries[j].temp == pastryTemp) return j;
        lastTypeMatch = j;
    }
    return lastTypeMatch;
}

} // namespace cafe
