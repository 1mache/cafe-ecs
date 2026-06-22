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
    bool       busy{ false };
    float      timer{ 0.f };                 // seconds elapsed this heating cycle
    PastryType cooking{ PastryType::count }; // type captured on intake (count = idle)
};

inline constexpr float HEAT_TIME          = 3.0f; // seconds to fully heat a pat
inline constexpr float HEATED_TEMPERATURE = 1.0f; // value written to Pastry.temperature when heated

inline constexpr float BAR_HEIGHT = 0.18f; // world half-height of the heating bar
inline constexpr float BAR_GAP    = 0.20f; // world gap between the bar and the machine top
} // namespace cafe

template <> struct bagel::Storage<cafe::Microwave> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::Microwave>;
};
