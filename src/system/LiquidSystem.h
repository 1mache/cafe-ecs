#pragma once

#include <SDL3/SDL.h>

namespace cafe
{
class AssetManager;
class PhysicsContext;

/** @brief Spawns drops from every active CoffeeSpawner, each of its own kind. */
void liquidSpawnerSystem(PhysicsContext& physics, float dtSeconds, AssetManager& assets);

/** @brief Drains Box2D sensor events: counts cup fills and destroys drops on contact. */
void liquidSensorEventSystem(PhysicsContext& physics);

/** @brief Prints a one-line debug summary every 0.5 s of accumulated dt. */
void dumpDebugStatsEvery(float dtSeconds);
} // namespace cafe
