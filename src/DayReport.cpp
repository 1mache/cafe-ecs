#include "DayReport.h"

#include "DayState.h"

namespace cafe
{
std::vector<ReportRow> buildReportRows()
{
    return {
        { "DAY",              std::to_string(DayState::dayNumber()) },
        { "",                 "" },
        { "SERVED",           std::to_string(DayState::served()) },
        { "LOST",             std::to_string(DayState::lost()) },
        { "SCORE",            std::to_string(DayState::score()) },
        { "",                 "" },
        { "CLICK TO CONTINUE", "" },
    };
}

bool tickDayClock(float& timeRemaining, float dt)
{
    if (timeRemaining <= 0.f) return true;
    timeRemaining -= dt;
    return timeRemaining <= 0.f;
}
} // namespace cafe
