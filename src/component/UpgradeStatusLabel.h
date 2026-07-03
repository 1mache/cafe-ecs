#pragma once

#include "UpgradeCatalog.h" // UpgradeId

#include <bagel.h>

namespace cafe
{
/** @brief Tag on the TextLabel entity showing one upgrade's "LVn cost/MAX"
 *  status, so DayReportScene::onUpdate can find and refresh it after a
 *  purchase. Deliberately separate from ShopButton: shopInputSystem's
 *  hit-test mask is {ShopButton, Transform} only, and this label must never
 *  become clickable. */
struct UpgradeStatusLabel
{
    UpgradeId id;
};
} // namespace cafe

template <> struct bagel::Storage<cafe::UpgradeStatusLabel> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::UpgradeStatusLabel>;
};
