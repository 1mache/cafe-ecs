#include <catch2/catch_test_macros.hpp>
#include "DayReport.h"
#include "DayState.h"

using namespace cafe;

TEST_CASE("tickDayClock counts down and signals day-over once it reaches zero")
{
    float t = 1.0f;
    REQUIRE(tickDayClock(t, 0.4f) == false);   // 0.6 left
    REQUIRE(tickDayClock(t, 0.4f) == false);   // 0.2 left
    REQUIRE(tickDayClock(t, 0.4f) == true);    // crosses 0
    REQUIRE(tickDayClock(t, 0.4f) == true);    // stays over
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
