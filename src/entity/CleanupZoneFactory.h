#pragma once

#include <bagel.h>

namespace cafe
{
class PhysicsContext;

/** @brief Static off-screen sensor that destroys spilled coffee drops. */
bagel::Entity createCleanupZone(PhysicsContext& physics);
} // namespace cafe
