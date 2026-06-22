#include "Menu.h"
#include "Utils.h"
#include <random>

namespace cafe
{
namespace
{
// Compile-time sanity: every recipe's ratio must sum to ~1.0 (catches typos).
constexpr bool ratiosSumToOne()
{
    for (const DrinkRecipe& r : MENU)
    {
float sum = 0.f;
        for (float x : r.ratio)
            sum += x;
        if (sum < 0.99f || sum > 1.01f)
            return false;
    }
    return true;
}
static_assert(ratiosSumToOne(), "each MENU ratio must sum to ~1.0");

int randInt(int below)
{
    std::uniform_int_distribution<int> dist(0, below - 1);
    return dist(getRng());
}
} // namespace

namespace
{
// Picks an allowed serving temperature for a drink (respects allowsHot/allowsCold).
Temperature randomDrinkTemp(const DrinkRecipe& r)
{
    if (r.allowsHot && r.allowsCold)
        return randInt(2) ? Temperature::Cold : Temperature::Hot;
    return r.allowsCold ? Temperature::Cold : Temperature::Hot;
}
} // namespace

Order randomOrder()
{
    Order o;
    o.drinkCount  = randInt(MAX_DRINKS + 1);    // 0..MAX_DRINKS
    o.pastryCount = randInt(MAX_PASTRIES + 1);  // 0..MAX_PASTRIES

    // Invariant: an order must have at least one item.
    if (o.drinkCount == 0 && o.pastryCount == 0)
    {
        if (randInt(2))
            o.drinkCount = 1;
        else
            o.pastryCount = 1;
    }

    for (int i = 0; i < o.drinkCount; ++i)
    {
        const auto d = static_cast<DrinkType>(randInt(static_cast<int>(DrinkType::count)));
        o.drinks[i] = { .type = d, .temp = randomDrinkTemp(recipeFor(d)) };
    }
    for (int i = 0; i < o.pastryCount; ++i)
    {
        const auto p = static_cast<PastryType>(randInt(static_cast<int>(PastryType::count)));
        o.pastries[i] = { .type = p,
                          .temp = randInt(2) ? Temperature::Cold : Temperature::Hot };
    }

    o.hasDrink  = o.drinkCount  > 0;
    o.hasPastry = o.pastryCount > 0;
    return o;
}

const char* pastryName(PastryType p)
{
    switch (p)
    {
    case PastryType::Croissant:    return "Croissant";
    case PastryType::CinnamonRoll: return "Cinnamon roll";
    case PastryType::Toast:        return "Toast";
    default:                       return "Unknown";
    }
}

const char* temperatureName(Temperature t)
{
    return t == Temperature::Cold ? "Cold" : "Hot";
}
} // namespace cafe
