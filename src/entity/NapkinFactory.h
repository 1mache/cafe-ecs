#pragma once

#include <bagel.h>

namespace cafe
{
class AssetManager;
class PhysicsContext;

/** @brief Napkin sprite anchored to the bottom-center of the screen. */
bagel::Entity createNapkin(AssetManager& assets, PhysicsContext& physics);
} // namespace cafe
