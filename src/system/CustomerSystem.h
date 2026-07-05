#pragma once

namespace cafe
{
class AssetManager;
class PhysicsContext;

/** @brief Keeps one customer at the seat: spawns the next after the seat empties. */
void customerSpawnerSystem(AssetManager& assets, PhysicsContext& physics, float dtSeconds);
void behaviorSystem(float dtSeconds);
/** @brief Maps each bubble's patience dial onto the props "patience" frames
 *  (full -> empty) from the owning customer's remaining patience. */
void patienceDialSystem(AssetManager& assets);
void deliverySystem();
void orderSystem();
void finalizeOrderGradeSystem();
/** @brief Spawn a graded text popup + particle burst at each leaving
 *  customer's position. Run after finalizeOrderGradeSystem (needs the final
 *  Behavior.rating) and before customerCleanupSystem (which destroys them). */
void gradePopupSystem();
void reportLeavingCustomers();
void customerCleanupSystem();
} // namespace cafe
