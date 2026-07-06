#include <catch2/catch_test_macros.hpp>
#include "UpgradeState.h"

using namespace cafe;

TEST_CASE("bankScore accumulates money")
{
    UpgradeState::resetAll();
    REQUIRE(UpgradeState::money() == 0);
    UpgradeState::bankScore(120);
    REQUIRE(UpgradeState::money() == 120);
    UpgradeState::bankScore(30);
    REQUIRE(UpgradeState::money() == 150);
}

TEST_CASE("tryBuy deducts cost and raises level when affordable")
{
    UpgradeState::resetAll();
    UpgradeState::bankScore(100);
    REQUIRE(UpgradeState::tryBuy(UpgradeId::TapSpeed) == true);   // costs 50
    REQUIRE(UpgradeState::level(UpgradeId::TapSpeed) == 1);
    REQUIRE(UpgradeState::money() == 50);
}

TEST_CASE("tryBuy fails when money is insufficient")
{
    UpgradeState::resetAll();
    UpgradeState::bankScore(40);                                  // < 50
    REQUIRE(UpgradeState::tryBuy(UpgradeId::TapSpeed) == false);
    REQUIRE(UpgradeState::level(UpgradeId::TapSpeed) == 0);
    REQUIRE(UpgradeState::money() == 40);
}

TEST_CASE("tryBuy unlocks KeyboardPour once, then it is maxed")
{
    UpgradeState::resetAll();
    UpgradeState::bankScore(25);
    REQUIRE(UpgradeState::level(UpgradeId::KeyboardPour) == 0);
    REQUIRE(UpgradeState::tryBuy(UpgradeId::KeyboardPour) == true);   // costs 10
    REQUIRE(UpgradeState::level(UpgradeId::KeyboardPour) == 1);
    REQUIRE(UpgradeState::money() == 15);
    REQUIRE(UpgradeState::tryBuy(UpgradeId::KeyboardPour) == false);  // maxed at 1
    REQUIRE(UpgradeState::money() == 15);
}

TEST_CASE("tryBuy unlocks KeyboardPastry once, then it is maxed")
{
    UpgradeState::resetAll();
    UpgradeState::bankScore(25);
    REQUIRE(UpgradeState::level(UpgradeId::KeyboardPastry) == 0);
    REQUIRE(UpgradeState::tryBuy(UpgradeId::KeyboardPastry) == true);   // costs 10
    REQUIRE(UpgradeState::level(UpgradeId::KeyboardPastry) == 1);
    REQUIRE(UpgradeState::money() == 15);
    REQUIRE(UpgradeState::tryBuy(UpgradeId::KeyboardPastry) == false);  // maxed at 1
    REQUIRE(UpgradeState::money() == 15);
}

TEST_CASE("tryBuy fails at max level")
{
    UpgradeState::resetAll();
    UpgradeState::bankScore(1000);
    REQUIRE(UpgradeState::tryBuy(UpgradeId::TapSpeed));  // lvl1: -50  -> 950
    REQUIRE(UpgradeState::tryBuy(UpgradeId::TapSpeed));  // lvl2: -75  -> 875
    REQUIRE(UpgradeState::tryBuy(UpgradeId::TapSpeed));  // lvl3: -100 -> 775
    REQUIRE(UpgradeState::level(UpgradeId::TapSpeed) == 3);
    REQUIRE(UpgradeState::tryBuy(UpgradeId::TapSpeed) == false);  // maxed
    REQUIRE(UpgradeState::level(UpgradeId::TapSpeed) == 3);
    REQUIRE(UpgradeState::money() == 775);
}
