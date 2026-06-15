#pragma once

namespace cafe
{
class AssetManager;
class PhysicsContext;

/** @brief Keeps one customer at the seat: spawns the next after the seat empties. */
void customerSpawnerSystem(PhysicsContext& physics, float dtSeconds, AssetManager& assets);
void behaviorSystem(float dtSeconds);
void deliverySystem();
void clearDeliveredItems();
void orderSystem();
void reportLeavingCustomers();
void customerCleanupSystem();
} // namespace cafe
