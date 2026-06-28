#pragma once

namespace cafe
{
/** @brief Ticks the DayClock entity; returns true when the day has ended. */
bool dayClockSystem(float dt);

/** @brief Reads Leaving+Behavior entities and records their results into DayState. */
void recordDayResultsSystem();
} // namespace cafe
