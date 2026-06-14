#include "Menu.h"
#include <cstdlib>
#include <ctime>

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

// Seed rand() once on first use so drinks vary between runs.
int randInt(int below)
{
    static const bool seeded = (std::srand(static_cast<unsigned>(std::time(nullptr))), true);
    (void)seeded;
    return std::rand() % below;
}
} // namespace

Order makeDrinkOrder(DrinkType d, Temperature t, bool hasPastry)
{
    return Order{ .drink = d, .drinkTemperature = t, .hasDrink = true, .hasPastry = hasPastry };
}

Order randomDrinkOrder(bool hasPastry)
{
    const auto d = static_cast<DrinkType>(randInt(static_cast<int>(DrinkType::count)));

    const DrinkRecipe& r = recipeFor(d);
    Temperature t = Temperature::Hot;
    if (r.allowsHot && r.allowsCold)
        t = randInt(2) ? Temperature::Cold : Temperature::Hot;
    else if (r.allowsCold)
        t = Temperature::Cold;

    Order o = makeDrinkOrder(d, t, hasPastry);
    if (hasPastry)
    {
        o.pastry = static_cast<PastryType>(randInt(static_cast<int>(PastryType::count)));
        o.pastryTemperature = randInt(2) ? Temperature::Cold : Temperature::Hot;
    }
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
