#pragma once

#include <bagel.h>

namespace cafe
{
class AssetManager;
class PhysicsContext;

/** @brief Keeps one customer at the seat: spawns the next after the seat empties. */
void customerSpawnerSystem(AssetManager& assets, float dtSeconds);
/** @brief Ticks patience while Ordering/Mad only (paused during the walk legs). */
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
/** @brief Drives Customer.state: attaches bubble/anim/sensor on arrival, times
 *  out the Mad sprite, and on Leaving strips deliverability, records the day
 *  result, logs the outcome, and starts the walk-out Tween. */
void customerStateSystem(AssetManager& assets, PhysicsContext& physics, float dt);
/** @brief If the customer is Ordering, switches it to Mad for CUSTOMER_MAD_TIME. */
void makeCustomerMad(bagel::Entity customer);
/** @brief If the customer is Mad, switches it back to Ordering immediately. */
void calmCustomer(bagel::Entity customer);
/** @brief Destroys a customer once its Leaving walk-out Tween has finished. */
void customerCleanupSystem();
} // namespace cafe
