#include "Menu.h"
#include "Utils.h"
#include <SDL3/SDL.h>
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
    // Each item is added only while it fits the bubble's icon budget, so the
    // order is built within the limit instead of generated then trimmed. A cold
    // drink or hot pastry costs 2 icons (item + temp), otherwise 1.
    int budget = MAX_ORDER_ICONS;

    int wantDrinks   = randInt(MAX_DRINKS + 1);    // 0..MAX_DRINKS
    int wantPastries = randInt(MAX_PASTRIES + 1);  // 0..MAX_PASTRIES
    // Invariant: an order must have at least one item.
    if (wantDrinks == 0 && wantPastries == 0)
    {
        if (randInt(2))
            wantDrinks = 1;
        else
            wantPastries = 1;
    }

    for (int i = 0; i < wantDrinks; ++i)
    {
        const auto d = static_cast<DrinkType>(randInt(static_cast<int>(DrinkType::count)));
        const Temperature t = randomDrinkTemp(recipeFor(d));
        const int cost = t == Temperature::Cold ? 2 : 1; // cold coffee requires 2 sprites to show that its cold
        if (cost > budget)
            break;
        o.drinks[o.drinkCount++] = { .type = d, .temp = t };
        budget -= cost;
    }
    for (int i = 0; i < wantPastries; ++i)
    {
        const auto p = static_cast<PastryType>(randInt(static_cast<int>(PastryType::count)));
        const Temperature t = randInt(2) ? Temperature::Cold : Temperature::Hot;
        const int cost = t == Temperature::Hot ? 2 : 1; // hot pastry requires 2 sprites to show that its hot
        if (cost > budget)
            break;
        o.pastries[o.pastryCount++] = { .type = p, .temp = t };
        budget -= cost;
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
    case PastryType::Bourekas:     return "Bourekas";
    case PastryType::Cheesecake:   return "Cheesecake";
    case PastryType::CarrotCake:   return "Carrot cake";
    default:                       return "Unknown";
    }
}

void validateOrderSprites(const SpriteSheet& props)
{
    const int pastrySprites = props.tagFrameCount("pastry");
    const int coffeeSprites = props.tagFrameCount("coffee");

    // enum bigger than sprites -> types we can't draw -> fatal
    assertFatal(static_cast<int>(PastryType::count) <= pastrySprites,
        "PastryType has more entries than props.json 'pastry' sprites");
    assertFatal(static_cast<int>(DrinkType::count) <= coffeeSprites,
        "DrinkType has more entries than props.json 'coffee' sprites");

    // sprites bigger than enum -> unimplemented art -> warning only
#ifdef DEBUG
    if (pastrySprites > static_cast<int>(PastryType::count))
        SDL_Log("Warning: props.json 'pastry' has %d sprites, only %d PastryType impl'd",
                pastrySprites, static_cast<int>(PastryType::count));
    if (coffeeSprites > static_cast<int>(DrinkType::count))
        SDL_Log("Warning: props.json 'coffee' has %d sprites, only %d DrinkType impl'd",
                coffeeSprites, static_cast<int>(DrinkType::count));
#endif
}

const char* temperatureName(Temperature t)
{
    return t == Temperature::Cold ? "Cold" : "Hot";
}
} // namespace cafe
