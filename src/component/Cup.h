#pragma once

#include "Ingredient.h"
#include <bagel.h>

namespace cafe
{
/** @brief A cup that catches liquid drops, tracked per ingredient. */
struct Cup
{
    int capacity{};
    int filled[INGREDIENT_COUNT]{}; // per-ingredient drop counts

    int totalFilled() const
    {
        int sum = 0;
        for (size_t i = 0; i < INGREDIENT_COUNT; ++i)
            sum += filled[i];
        return sum;
    }
    float fillPercent() const
    {
        return capacity ? static_cast<float>(totalFilled()) / static_cast<float>(capacity) : 0.f;
    }
    bool isFull() const { return totalFilled() >= capacity; }
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Cup> final : NoInstance { using type = SparseStorage<cafe::Cup>; };