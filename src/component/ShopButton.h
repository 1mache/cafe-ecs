#pragma once

#include "UpgradeCatalog.h"
#include <bagel.h>

namespace cafe
{
/** @brief A clickable shop entry (UI sprite: Transform + Drawable + ShopButton).
 *  `justPressed` is set by shopInputSystem when a click lands inside the entity's
 *  Transform, and consumed by shopPurchaseSystem. Mirrors the SpawnButton idiom. */
struct ShopButton
{
    UpgradeId id{};
    bool      justPressed{};
};
} // namespace cafe

template <> struct bagel::Storage<cafe::ShopButton> final : NoInstance
{ using type = SparseStorage<cafe::ShopButton>; };
