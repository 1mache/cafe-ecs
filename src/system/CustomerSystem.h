#pragma once

namespace cafe
{
class AssetManager;

/** @brief Keeps one customer at the seat: spawns the next after the seat empties. */
void customerSpawnerSystem(float dtSeconds, AssetManager& assets);
void behaviorSystem(float dtSeconds);
void deliverySystem();
void orderSystem();
void reportLeavingCustomers();
void customerCleanupSystem();
} // namespace cafe
