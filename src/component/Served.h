#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Tracks what a client has been served so far. */
struct Served
{
    bool drink{};
    bool pastry{};
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Served> final : NoInstance { using type = SparseStorage<cafe::Served>; };
