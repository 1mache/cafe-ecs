#pragma once

namespace cafe
{
/** @brief A named beverage on the menu. `count` is a sentinel (must stay last). */
enum class DrinkType { Espresso = 0, Americano, Cappuccino, Latte, Macchiato, count };

/** @brief Serving temperature of a drink. Carried on an order; not graded for now. */
enum class Temperature { Hot, Cold };
} // namespace cafe
