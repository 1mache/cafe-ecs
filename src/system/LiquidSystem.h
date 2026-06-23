#pragma once

#include <SDL3/SDL.h>

namespace cafe
{
class AssetManager;
class PhysicsContext;

/** @brief Spawns drops from every active CoffeeSpawner, each of its own kind. */
void liquidSpawnerSystem(AssetManager& assets, PhysicsContext& physics, float dtSeconds);

void liquidVelocityClampSystem();

/** @brief Drains Box2D sensor events: counts cup fills and destroys drops on contact. */
void liquidSensorEventSystem(PhysicsContext& physics);
} // namespace cafe
