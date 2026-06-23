#include "TimerBarSystem.h"

#include "Components.h"
#include <bagel.h>
#include <algorithm>

namespace cafe
{
namespace
{
// 0..1 progress for a bar's source entity, or -1 if the source isn't timed.
float sourceFraction(bagel::Entity src)
{
    if (src.has<Microwave>())
    {
        const auto& mw = src.get<Microwave>();
        return mw.busy ? std::clamp(mw.timer / HEAT_TIME, 0.f, 1.f) : 0.f;
    }
    if (src.has<DayClock>())
    {
        const auto& dc = src.get<DayClock>();
        if (dc.dayLength <= 0.f) return 0.f;
        return std::clamp(dc.timeRemaining / dc.dayLength, 0.f, 1.f);
    }
    return -1.f;
}
} // namespace

void timerBarSystem()
{
    static const bagel::Mask barMask =
        bagel::MaskBuilder().set<TimerBar>().set<Transform>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(barMask)) continue;

        bagel::Entity src = e.get<TimerBar>().source;
        if (!src.has<Transform>()) continue;

        const float frac = sourceFraction(src);
        if (frac < 0.f) continue; // unrecognized source

        const auto& st = src.get<Transform>(); // source (machine or day-clock anchor)
        auto&       bt = e.get<Transform>();    // this bar

        // Left edge anchored to the source's left edge; width tracks the fraction.
        // Height was set once by the factory; only width/position change here.
        bt.w = st.w * frac;
        bt.x = (st.x - st.w) + bt.w;
        bt.y = st.y + st.h + BAR_GAP;
    }
}
} // namespace cafe
