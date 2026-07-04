#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief Tag for the menu's START square, so it flows through the same
 *  Transform hit-test as NextDayButton. A hit ends the scene immediately
 *  (menuInputSystem sets an out-param), so no justPressed consume step. */
struct StartButton {};
} // namespace cafe

template <> struct bagel::Storage<cafe::StartButton> final : NoInstance
{ using type = SparseStorage<cafe::StartButton>; };
