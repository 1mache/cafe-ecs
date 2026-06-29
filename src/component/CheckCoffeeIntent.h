#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Cup was dropped on a customer: target customer for grading. */
struct CheckCoffeeIntent
{
    bagel::ent_type customer{ -1 };
};
} // namespace cafe

template <> struct bagel::Storage<cafe::CheckCoffeeIntent> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::CheckCoffeeIntent>;
};
