#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief A client's runtime patience and satisfaction rating. */
struct Behavior
{
    float patience{}; // seconds remaining before the client leaves
    int   rating{};   // satisfaction score written after order is served
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Behavior> final : NoInstance { using type = SparseStorage<cafe::Behavior>; };
