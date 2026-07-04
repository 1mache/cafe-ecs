#pragma once

namespace cafe
{
class AssetManager;
class PhysicsContext;

/** @brief Drives napkin position (Box2D velocity chase) and size (exponential ease)
 *  toward the layout for NapkinIntent.state. Run before PhysicsContext::step(). */
void napkinSystem(AssetManager& assets, PhysicsContext& physics, float dt);
} // namespace cafe
