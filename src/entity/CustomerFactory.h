#pragma once

#include "Animation.h"
#include "Order.h"
#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
class AssetManager;
class PhysicsContext;

/** @brief 2-frame (mouth shut/open) talking loop for customer sheet frame
 *  `customerId`; ends after TALKING_TIME via its globalTimer. */
Animation createTalkingAnimation(int customerId);

/** @brief Spawns a customer at CUSTOMER_ENTRANCE, already Tweening in toward
 *  `seat`. No bubble, talking anim, or drop-space sensor yet — customerStateSystem
 *  attaches those on arrival (see attachOrderBubble / makeCustomerDeliverable). */
bagel::Entity createCustomer(AssetManager& assets, WorldPos seat, const Order order, float patience);

/** @brief Attaches the speech bubble, order icons, checkmarks, and patience dial
 *  to an already-seated customer. Returns the bubble entity (stash it in
 *  Customer.bubble so departure can Destroy the whole subtree). */
bagel::Entity attachOrderBubble(AssetManager& assets, bagel::Entity customer);

/** @brief Attaches a static DropSpace sensor + OrderGrade to a seated customer so
 *  dragged items can be "dropped on" them. Call once, on arrival. */
void makeCustomerDeliverable(PhysicsContext& physics, bagel::Entity client);

/** @brief Sets a customer's Drawable to the given sheet frame (e.g.
 *  Customer.spriteBase for idle, +2 for mad). */
void setCustomerFrame(AssetManager& assets, bagel::Entity customer, int frameIndex);
} // namespace cafe
