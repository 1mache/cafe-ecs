#pragma once

namespace cafe
{
/** @brief A named beverage on the menu. `count` is a sentinel (must stay last). */
enum class DrinkType
{
    Black,
    Espresso,
    Americano,
    Latte,
    Cappuccino,
    count
};

/** @brief A pastry on the menu. Value == its props.png frame index.
 *  `count` is a sentinel (must stay last). */
enum class PastryType
{
    Croissant = 0,
    CinnamonRoll,
    Bourekas,
    Cheesecake,
    CarrotCake,
    count
};

/** @brief Serving temperature of an item. */
enum class Temperature { Hot, Cold };
} // namespace cafe
