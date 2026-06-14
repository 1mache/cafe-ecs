#pragma once

namespace cafe
{
/** @brief A pastry on the menu. Value == its props.png frame index.
 *  `count` is a sentinel (must stay last). */
enum class PastryType { Croissant = 0, CinnamonRoll, Toast, count };
} // namespace cafe
