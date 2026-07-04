#pragma once

namespace cafe
{
/** @brief Drift + fade every Particle entity by dt (moves Transform, scales
 *  Drawable/TextLabel tint alpha across its Lifetime). */
void particleSystem(float dt);

/** @brief Age every Lifetime entity by dt; destroy those past their duration
 *  (via destroyPhysicalEntity). Run AFTER particleSystem so the final frame
 *  still draws. */
void lifetimeSystem(float dt);
} // namespace cafe
