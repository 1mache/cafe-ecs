#pragma once

namespace cafe
{
/** @brief Resize every TimerBar's Transform from its source's progress fraction.
 *  Handles Microwave sources (timer/HEAT_TIME, 0 when idle) and DayClock sources
 *  (timeRemaining/dayLength). The bar is left-anchored to the source's Transform.
 *  Run once per frame before drawSystem. Replaces the old microwaveBarSystem. */
void timerBarSystem();
} // namespace cafe
