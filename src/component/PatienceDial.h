#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Marks the bubble's patience dial overlay. patienceDialSystem maps the
 *  owning customer's remaining patience onto the props "patience" frames. The
 *  customer is found by walking the ChildOf chain (dial -> bubble -> customer). */
struct PatienceDial
{
};
} // namespace cafe

template <> struct bagel::Storage<cafe::PatienceDial> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::PatienceDial>;
};