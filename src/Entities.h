#pragma once
#include "Components.h"
#include "WorldPos.h"
#include <bagel.h>

namespace cafe
{

/** @brief Creates a client entity with Transform, Drawable, Order, and Behavior components.
 *  @param tex      Pre-loaded texture (not owned here).
 *  @param texW     Texture width in pixels.
 *  @param texH     Texture height in pixels.
 *  @param pos      Spawn position in world units (meters, Y-up).
 *  @param order    The client's order. Invariant: hasDrink || hasPastry.
 *  @param patience Time in seconds before the client leaves without being served.
 */
bagel::Entity createClient(SDL_Texture* tex, float texW, float texH,
                           WorldPos pos, const Order& order, float patience);

} // namespace cafe