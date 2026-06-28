#include <catch2/catch_test_macros.hpp>
#include "DayState.h"

using namespace cafe;

TEST_CASE("DayState records served, lost, and summed score")
{
    DayState::resetAll();
    DayState::beginNewDay();            // day 1
    DayState::record(true, 195);
    DayState::record(false, 40);        // failed but partial credit
    DayState::record(true, 100);

    REQUIRE(DayState::dayNumber() == 1);
    REQUIRE(DayState::served() == 2);
    REQUIRE(DayState::lost() == 1);
    REQUIRE(DayState::score() == 335);
}

TEST_CASE("beginNewDay increments the day and resets the tally")
{
    DayState::resetAll();
    DayState::beginNewDay();
    DayState::record(true, 50);
    DayState::beginNewDay();            // day 2

    REQUIRE(DayState::dayNumber() == 2);
    REQUIRE(DayState::served() == 0);
    REQUIRE(DayState::lost() == 0);
    REQUIRE(DayState::score() == 0);
}
