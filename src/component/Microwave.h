#pragma once

#include "Pastry.h"
#include <bagel.h>

namespace cafe
{
/** @brief A microwave that heats one pastry at a time.
 *  busy => a pat is inside; timer counts up to HEAT_TIME; cooking is the
 *  PastryType captured on intake, re-spawned (heated) on spit-out. */
struct Microwave
{
    bool          busy{ false };
    float         timer{ 0.f };                 // seconds elapsed this heating cycle
    PastryType    cooking{ PastryType::count }; // type captured on intake (count = idle)
    bagel::Entity display{ bagel::ent_type(-1) }; // child number-display entity; invalid while idle
    bagel::Entity glow{ bagel::ent_type(-1) };    // child glow overlay; invalid while idle
};

inline constexpr auto OVEN_TEX              = "oven.png";
inline constexpr auto OVEN_SPRITE_DATA      = "oven.json";
inline constexpr int  OVEN_DEFAULT_SPRITE_ID = 0;
inline constexpr int  OVEN_COOKING_SPRITE_ID = 1;
inline constexpr int  OVEN_GLOW_SPRITE_ID    = 2;

inline constexpr auto OVEN_NUMBERS_TEX  = "oven_numbers.png";
inline constexpr auto OVEN_NUMBERS_DATA = "oven_numbers.json";

inline constexpr float HEAT_TIME = 5.5f; // seconds to fully heat a pat
static_assert(HEAT_TIME > 0.f && HEAT_TIME < 5.9f, "Only have microwave numbers 0-5");

inline constexpr float BAR_GAP = 0.20f; // world gap between the bar and the machine top (day bar)
} // namespace cafe

template <> struct bagel::Storage<cafe::Microwave> final : NoInstance
{
    using type = SparseStorage<cafe::Microwave>;
};
