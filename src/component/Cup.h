#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief A cup that catches coffee drops. */
struct Cup
{
    int   capacity{};
    int   filled{};
    float fillPercent() const
    {
        return capacity ? static_cast<float>(filled) / static_cast<float>(capacity) : 0.f;
    }
    bool  isFull() const { return filled >= capacity; }
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Cup> final : NoInstance { using type = SparseStorage<cafe::Cup>; };
