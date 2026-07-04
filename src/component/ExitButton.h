#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Tag for the menu's EXIT square. See StartButton — same idiom. */
struct ExitButton {};
} // namespace cafe

template <> struct bagel::Storage<cafe::ExitButton> final : NoInstance
{ using type = SparseStorage<cafe::ExitButton>; };
