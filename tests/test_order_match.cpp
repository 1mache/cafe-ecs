#include "OrderMatch.h"
#include <catch2/catch_test_macros.hpp>

using namespace cafe;

namespace
{
// Build a Cup with the given per-ingredient counts (capacity irrelevant to grading).
Cup cupWith(int coffee, int milk, int water)
{
    Cup c{};
    c.capacity = 100;
    c.filled[static_cast<size_t>(Ingredient::Coffee)] = coffee;
    c.filled[static_cast<size_t>(Ingredient::Milk)]   = milk;
    c.filled[static_cast<size_t>(Ingredient::Water)]  = water;
    return c;
}

Order orderWith(int coffee, int milk, int water)
{
    return Order{ .ratio = { coffee, milk, water }, .hasDrink = true, .hasPastry = false };
}
} // namespace

TEST_CASE("Exact ratio match grades Perfect", "[ordermatch]")
{
    // Order 1:1 coffee:milk, cup 25:25 → identical fractions.
    REQUIRE(gradeDrinkRatio(orderWith(1, 1, 0), cupWith(25, 25, 0)) == DrinkGrade::Perfect);
    // Scale-invariant: order 3:7 vs cup 30:70.
    REQUIRE(gradeDrinkRatio(orderWith(3, 7, 0), cupWith(30, 70, 0)) == DrinkGrade::Perfect);
}

TEST_CASE("Close-but-imperfect ratio grades Acceptable (43/57 ~= 50/50)", "[ordermatch]")
{
    // Order 50/50, cup 43/57 → max per-ingredient diff 0.07, inside the Acceptable band.
    REQUIRE(gradeDrinkRatio(orderWith(1, 1, 0), cupWith(43, 57, 0)) == DrinkGrade::Acceptable);
}

TEST_CASE("Far-off ratio grades Wrong", "[ordermatch]")
{
    // Order pure coffee, cup pure milk.
    REQUIRE(gradeDrinkRatio(orderWith(1, 0, 0), cupWith(0, 50, 0)) == DrinkGrade::Wrong);
    // Order has no water, but the cup is mostly water.
    REQUIRE(gradeDrinkRatio(orderWith(1, 1, 0), cupWith(10, 10, 80)) == DrinkGrade::Wrong);
}

TEST_CASE("Empty cup or empty order grades Wrong", "[ordermatch]")
{
    REQUIRE(gradeDrinkRatio(orderWith(1, 1, 0), cupWith(0, 0, 0)) == DrinkGrade::Wrong);
    REQUIRE(gradeDrinkRatio(orderWith(0, 0, 0), cupWith(25, 25, 0)) == DrinkGrade::Wrong);
}