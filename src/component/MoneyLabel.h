#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Tag on the TextLabel entity showing the current money balance, so
 *  DayReportScene::onUpdate can find and refresh it after a purchase. */
struct MoneyLabel {};
} // namespace cafe

template <> struct bagel::Storage<cafe::MoneyLabel> final : bagel::NoInstance
{
    using type = bagel::TaggedStorage<cafe::MoneyLabel>;
};
