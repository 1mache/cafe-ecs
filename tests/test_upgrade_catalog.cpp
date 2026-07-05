#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "UpgradeCatalog.h"

using namespace cafe;

TEST_CASE("upgradeCost is baseCost + currentLevel*costStep")
{
    REQUIRE(upgradeCost(UpgradeId::TapSpeed, 0) == 50);
    REQUIRE(upgradeCost(UpgradeId::TapSpeed, 1) == 75);
    REQUIRE(upgradeCost(UpgradeId::TapSpeed, 2) == 100);
    REQUIRE(upgradeCost(UpgradeId::MicrowaveSpeed, 0) == 60);
    REQUIRE(upgradeCost(UpgradeId::MicrowaveSpeed, 2) == 120);
}

TEST_CASE("upgradeValue is baseValue + level*valueStep")
{
    REQUIRE(upgradeValue(UpgradeId::TapSpeed, 0) == Catch::Approx(0.05f));
    REQUIRE(upgradeValue(UpgradeId::TapSpeed, 3) == Catch::Approx(0.014f));
    REQUIRE(upgradeValue(UpgradeId::MicrowaveSpeed, 0) == Catch::Approx(5.5f));
    REQUIRE(upgradeValue(UpgradeId::MicrowaveSpeed, 3) == Catch::Approx(1.6f));
}

TEST_CASE("isMaxed is true at or beyond maxLevel")
{
    REQUIRE(isMaxed(UpgradeId::TapSpeed, 2) == false);
    REQUIRE(isMaxed(UpgradeId::TapSpeed, 3) == true);
    REQUIRE(isMaxed(UpgradeId::TapSpeed, 4) == true);
}

TEST_CASE("KeyboardPour is a binary unlock costing 10")
{
    REQUIRE(upgradeCost(UpgradeId::KeyboardPour, 0) == 10);
    REQUIRE(isMaxed(UpgradeId::KeyboardPour, 0) == false);
    REQUIRE(isMaxed(UpgradeId::KeyboardPour, 1) == true);
}
