#pragma once

namespace cafe
{
/** @brief Returns true once served+lost customers reach the DayProgress target. */
bool dayEndSystem();

/** @brief Reads Leaving+Behavior entities and records their results into DayState. */
void recordDayResultsSystem();
} // namespace cafe
