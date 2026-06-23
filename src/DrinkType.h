#pragma once

namespace cafe
{
/** @brief A named beverage on the menu. `count` is a sentinel (must stay last). */
enum class DrinkType { Black, Espresso, Americano, Latte, Cappuccino, count };

/** @brief Serving temperature of an item. */
enum class Temperature { Hot, Cold };
} // namespace cafe
