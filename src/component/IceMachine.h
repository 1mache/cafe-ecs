#pragma once

#include <bagel.h>

namespace cafe
{
// TODO: delete
/** @brief Tag marking the ice machine entity. Rendered as a placeholder gray
 *  square by drawSystem; its on-machine spawn button drops ice cubes. */
struct IceMachine {};
} // namespace cafe

template <> struct bagel::Storage<cafe::IceMachine> final : NoInstance { using type = TaggedStorage<cafe::IceMachine>; };
