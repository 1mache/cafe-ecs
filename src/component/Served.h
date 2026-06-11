#pragma once

#include "OrderMatch.h"
#include <bagel.h>

namespace cafe
{
/** @brief Tracks what a client has been served so far. */
struct Served
{
    bool       drink{};
    bool       pastry{};
    DrinkGrade drinkGrade{ DrinkGrade::Wrong }; // quality of the delivered drink
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Served> final : NoInstance { using type = SparseStorage<cafe::Served>; };