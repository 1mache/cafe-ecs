#include "DayCycleSystem.h"
#include "Components.h"
#include "DayReport.h"
#include "DayState.h"
#include <bagel.h>

namespace cafe
{

bool dayClockSystem(float dt)
{
    static const bagel::Mask clockMask =
        bagel::MaskBuilder().set<DayClock>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(clockMask)) continue;

        auto& clock = e.get<DayClock>();
        if (tickDayClock(clock.timeRemaining, dt))
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
