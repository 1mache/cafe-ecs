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

/** @brief Creates a speech bubble entity that follows @p parent via ChildOf.
 *  @param tex       Pre-loaded bubble texture (not owned here).
 *  @param texW      Texture width in pixels.
 *  @param texH      Texture height in pixels.
 *  @param parent    Entity this bubble tracks (the client).
 *  @param offsetPx  Offset from parent center in screen pixels (Y-down convention).
 */
bagel::Entity createSpeechBubble(SDL_Texture* tex, float texW, float texH,
                                 bagel::Entity parent, SDL_FPoint offsetPx);

/** @brief Creates a small icon entity inside a speech bubble.
 *  @param tex            Pre-loaded sprite sheet texture (not owned here).
 *  @param srcRect        Sub-rect of the texture to display.
 *  @param parentBubble   The bubble entity this icon is a child of.
 *  @param offsetPx       Offset from bubble center in screen pixels.
 */
bagel::Entity createOrderIcon(SDL_Texture* tex, SDL_FRect srcRect,
                              bagel::Entity parentBubble, SDL_FPoint offsetPx);

} // namespace cafe
