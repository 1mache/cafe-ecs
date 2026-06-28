#pragma once

#include <string>
#include <vector>

namespace cafe
{
/** @brief One line of the end-of-day receipt. `value` empty => header/spacer row. */
struct ReportRow
{
    std::string label;
    std::string value;
};

/** @brief Build the receipt rows from the current DayState. Add a future stat by
 *  adding a DayState field and pushing one more row here. */
std::vector<ReportRow> buildReportRows();

/** @brief Decrement @p timeRemaining by @p dt; return true once it reaches 0. */
bool tickDayClock(float& timeRemaining, float dt);
} // namespace cafe
