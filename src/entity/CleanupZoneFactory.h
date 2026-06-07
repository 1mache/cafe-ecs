#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Static off-screen sensor that destroys spilled coffee drops. */
bagel::Entity createCleanupZone();
} // namespace cafe
