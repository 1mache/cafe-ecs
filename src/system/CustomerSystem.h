#pragma once

namespace cafe
{
class AssetManager;
class PhysicsContext;

/** @brief Keeps one customer at the seat: spawns the next after the seat empties. */
void customerSpawnerSystem(AssetManager& assets, PhysicsContext& physics, float dtSeconds);
void behaviorSystem(float dtSeconds);
void deliverySystem();
void orderSystem();
void finalizeOrderGradeSystem();
void reportLeavingCustomers();
void customerCleanupSystem();
} // namespace cafe
