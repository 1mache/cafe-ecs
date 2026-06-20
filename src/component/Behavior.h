#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief A client's runtime patience and satisfaction rating. */
struct Behavior
{
    float patience{};    // seconds remaining before the client leaves
    float maxPatience{}; // initial patience, used for penalty thresholds
    int   rating{};      // penalized final grade written in finalizeOrderGradeSystem
    bool  succeeded{};   // true if all ordered items were received before timeout
};
} // namespace cafe

template <> struct bagel::Storage<cafe::Behavior> final : NoInstance { using type = SparseStorage<cafe::Behavior>; };
