#pragma once
#include "WorldPos.h"
#include <functional>

namespace cafe
{
void drawSystem();

/** @brief Copies each entity's b2Body position/rotation into its Transform. */
void syncTransformFromBody();

/** @brief Spawns coffee drops from every active CoffeeSpawner. */
void coffeeSpawnerSystem(float dtSeconds);

/** @brief Same as coffeeSpawnerSystem but emits through @p spawnDrop — for tests without Box2D. */
void coffeeSpawnerSystemImpl(float dtSeconds,
                             const std::function<void(WorldPos)>& spawnDrop);

/** @brief Drains Box2D sensor events: counts cup fills and destroys drops on contact. */
void sensorEventSystem();

/** @brief Prints a one-line debug summary every 0.5 s of accumulated dt. */
void dumpDebugStatsEvery(float dtSeconds);
void hierarchySystem();
}
