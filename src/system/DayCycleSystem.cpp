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

} // namespace cafe
