#include <catch2/catch_test_macros.hpp>
#include "DayReport.h"
#include "DayState.h"

using namespace cafe;

TEST_CASE("dayIsOver is true once handled reaches the target")
{
    REQUIRE(dayIsOver(0, 10) == false);
    REQUIRE(dayIsOver(9, 10) == false);
    REQUIRE(dayIsOver(10, 10) == true);   // boundary: exactly the target
    REQUIRE(dayIsOver(11, 10) == true);
}

TEST_CASE("buildReportRows reflects the current DayState")
{
    DayState::resetAll();
    DayState::beginNewDay();           // day 1
    DayState::record(true, 100);
    DayState::record(false, 20);

    const auto rows = buildReportRows();

    REQUIRE(rows[0].label == "DAY");
    REQUIRE(rows[0].value == "1");
    REQUIRE(rows[2].label == "SERVED");
    REQUIRE(rows[2].value == "1");
    REQUIRE(rows[3].label == "LOST");
    REQUIRE(rows[3].value == "1");
    REQUIRE(rows[4].label == "SCORE");
    REQUIRE(rows[4].value == "120");
}
