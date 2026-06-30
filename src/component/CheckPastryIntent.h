#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Pastry was dropped on a customer: target customer for grading. */
struct CheckPastryIntent
{
    bagel::ent_type customer{ -1 };
};
} // namespace cafe

template <> struct bagel::Storage<cafe::CheckPastryIntent> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::CheckPastryIntent>;
};
