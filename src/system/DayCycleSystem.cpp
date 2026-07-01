#include "DayCycleSystem.h"
#include "Components.h"
#include "DayReport.h"
#include "DayState.h"
#include <bagel.h>

namespace cafe
{

bool dayEndSystem()
{
    static const bagel::Mask progressMask =
        bagel::MaskBuilder().set<DayProgress>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(progressMask)) continue;

        const auto& dp = e.get<DayProgress>();
        if (dayIsOver(DayState::served() + DayState::lost(), dp.target))
            return true;
    }
    return false;
}

void recordDayResultsSystem()
{
    static const bagel::Mask leavingBehaviorMask =
        bagel::MaskBuilder().set<Leaving>().set<Behavior>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(leavingBehaviorMask)) continue;

        const auto& b = e.get<Behavior>();
        DayState::record(b.succeeded, b.rating);
    }
}

} // namespace cafe
