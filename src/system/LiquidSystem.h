#pragma once

#include "Ingredient.h"
#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <functional>

namespace cafe
{
class AssetManager;

/** @brief Copies each pipe's PourIntent onto its CoffeeSpawner.active. */
void pourControlSystem();

/** @brief Spawns drops from every active CoffeeSpawner, each of its own kind. */
void coffeeSpawnerSystem(float dtSeconds, AssetManager& assets);

/** @brief Same as coffeeSpawnerSystem but emits through @p spawnDrop — for tests without Box2D. */
void coffeeSpawnerSystemImpl(float dtSeconds,
                             const std::function<void(WorldPos, Ingredient)>& spawnDrop);

/** @brief Drains Box2D sensor events: counts cup fills and destroys drops on contact. */
void liquidSensorEventSystem();

/** @brief Prints a one-line debug summary every 0.5 s of accumulated dt. */
void dumpDebugStatsEvery(float dtSeconds);
} // namespace cafe
