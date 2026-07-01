#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Singleton-style component: one entity holds this to define how many
 *  customers (served + lost) end the day. The live count lives in DayState. */
struct DayProgress
{
    int target{};
};
} // namespace cafe

template <> struct bagel::Storage<cafe::DayProgress> final : NoInstance { using type = SparseStorage<cafe::DayProgress>; };
