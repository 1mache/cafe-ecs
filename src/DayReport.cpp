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

bool dayIsOver(int handled, int target)
{
    return handled >= target;
}
} // namespace cafe
