#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Tag for the "start next day" button entity, so it flows through the
 *  same Transform hit-test as ShopButton without carrying an UpgradeId. */
struct NextDayButton {};
} // namespace cafe

template <> struct bagel::Storage<cafe::NextDayButton> final : NoInstance
{ using type = SparseStorage<cafe::NextDayButton>; };
