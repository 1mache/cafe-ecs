#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Singleton-style component: one entity holds this to drive the day timer. */
struct DayClock
{
    float timeRemaining{};
    float dayLength{};
};
} // namespace cafe

template <> struct bagel::Storage<cafe::DayClock> final : NoInstance { using type = SparseStorage<cafe::DayClock>; };
