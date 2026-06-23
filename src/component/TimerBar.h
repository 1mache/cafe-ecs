#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief A progress bar entity whose width tracks another entity's progress.
 *  source is the entity being measured (e.g. a Microwave or a DayClock);
 *  timerBarSystem reads its progress each frame and resizes this bar's
 *  Transform (width 0 = hidden). */
struct TimerBar
{
    bagel::Entity source{ bagel::ent_type(-1) };
};
} // namespace cafe

template <> struct bagel::Storage<cafe::TimerBar> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::TimerBar>;
};
