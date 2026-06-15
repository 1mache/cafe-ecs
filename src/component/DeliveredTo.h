#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Marks an item as handed to a specific customer (sitting in their drop
 *  zone). When that customer leaves, all their delivered items are destroyed. */
struct DeliveredTo { bagel::ent_type customer{ -1 }; };
} // namespace cafe

template <> struct bagel::Storage<cafe::DeliveredTo> final : bagel::NoInstance { using type = bagel::SparseStorage<cafe::DeliveredTo>; };
