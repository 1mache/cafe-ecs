#include <catch2/catch_test_macros.hpp>
#include "OrderMatch.h"

using namespace cafe;

static CheckCoffeeIntent blackCoffeeIntent()
{
    return CheckCoffeeIntent{
        .ratio      = { 0.80f, 0.20f, 0.00f },
        .targetFill = 1.0f,
    };
}

static CoffeeOverview perfectBlackCoffeeOverview(bool isHot = false)
{
    return CoffeeOverview{
        .ratio       = { 0.80f, 0.20f, 0.00f },
        .isHot       = isHot,
        .dropSum     = 100,
        .fillPercent = 1.0f,
    };
}

static CoffeeOverview pureCoffeeOverview(bool isHot = false)
{
    return CoffeeOverview{
        .ratio       = { 1.00f, 0.00f, 0.00f },
        .isHot       = isHot,
        .dropSum     = 100,
        .fillPercent = 1.0f,
    };
}

TEST_CASE("gradeDrink returns 0 for empty cup")
{
    const CheckCoffeeIntent intent = blackCoffeeIntent();
    const CoffeeOverview  overview{};

    REQUIRE(gradeDrink(intent, overview) == 0);
}

TEST_CASE("gradeDrink scores perfect black coffee at 100")
{
    const CheckCoffeeIntent intent = blackCoffeeIntent();
    const CoffeeOverview  overview = perfectBlackCoffeeOverview();

    REQUIRE(gradeDrink(intent, overview) == 100);
}

TEST_CASE("gradeDrink forgives compositional ratio error for black coffee served as pure coffee")
{
    CheckCoffeeIntent intent = blackCoffeeIntent();
    intent.isHot             = true;
    const CoffeeOverview overview = pureCoffeeOverview(true);

    REQUIRE(gradeDrink(intent, overview) == 84);
}

TEST_CASE("gradeDrink scores perfect cold drink at 100 with cold match bonus")
{
    CheckCoffeeIntent intent = blackCoffeeIntent();
    intent.isHot             = false;
    const CoffeeOverview overview = perfectBlackCoffeeOverview(false);

    REQUIRE(gradeDrink(intent, overview) == 100);
}

TEST_CASE("gradeDrink scores perfect hot drink at 100 with no temperature bonus")
{
    CheckCoffeeIntent intent = blackCoffeeIntent();
    intent.isHot             = true;
    const CoffeeOverview overview = perfectBlackCoffeeOverview(true);

    REQUIRE(gradeDrink(intent, overview) == 100);
}

TEST_CASE("gradeDrink halves score on temperature mismatch")
{
    CheckCoffeeIntent intent = blackCoffeeIntent();
    intent.isHot             = true;
    const CoffeeOverview overview = perfectBlackCoffeeOverview(false);

    REQUIRE(gradeDrink(intent, overview) == 50);
}

TEST_CASE("gradeDrink applies cold match bonus to imperfect drink capped at 100")
{
    CheckCoffeeIntent intent = blackCoffeeIntent();
    intent.isHot             = false;
    const CoffeeOverview overview = pureCoffeeOverview(false);

    REQUIRE(gradeDrink(intent, overview) == 100);
}

TEST_CASE("gradeDrink halves imperfect drink score on temperature mismatch")
{
    CheckCoffeeIntent intent = blackCoffeeIntent();
    intent.isHot             = true;
    const CoffeeOverview overview = pureCoffeeOverview(false);

    REQUIRE(gradeDrink(intent, overview) == 42);
}
