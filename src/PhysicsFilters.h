#pragma once
#include <cstdint>

// Box2D collision categories shared across the project.
// Each mask only matches bits this feature controls.

namespace cafe::filter
{

enum FilterType: int
{
    LIQUID_BIT = 0,
    CUP_SOLID_BIT,
    CUP_INSIDE_BIT,
    CUP_LID_BIT,
    CLEANUP_BIT,
    DROPSPACE_SENSOR_BIT,
    DRAGGABLE_BIT,
    FURNITURE_BIT,
    ICE_BIT,
    count
};

constexpr uint64_t bit2Filter(int bit)
{
    return 1ull << bit;
}

inline constexpr uint64_t LIQUID           = bit2Filter(LIQUID_BIT); // coffee drops
inline constexpr uint64_t CUP_SOLID        = bit2Filter(CUP_SOLID_BIT); // cup walls & bottom
inline constexpr uint64_t CUP_INSIDE       = bit2Filter(CUP_INSIDE_BIT); // cup interior sensor
inline constexpr uint64_t CUP_LID          = bit2Filter(CUP_LID_BIT); // prevents cups from going into each other
inline constexpr uint64_t CLEANUP          = bit2Filter(CLEANUP_BIT); // out-of-bounds sensor
inline constexpr uint64_t DROPSPACE_SENSOR = bit2Filter(DROPSPACE_SENSOR_BIT); // drop-zone sensor
inline constexpr uint64_t DRAGGABLE        = bit2Filter(DRAGGABLE_BIT); // draggable visitor shape
inline constexpr uint64_t FURNITURE        = bit2Filter(FURNITURE_BIT); // solid furniture
inline constexpr uint64_t ICE              = bit2Filter(ICE_BIT); // ice cube: rests on furniture, caught by cup

inline constexpr uint64_t MASK_LIQUID           = CUP_SOLID | ICE | CUP_INSIDE | CLEANUP;
inline constexpr uint64_t MASK_CUP_SOLID        = LIQUID | FURNITURE | CUP_SOLID | DRAGGABLE | ICE;
inline constexpr uint64_t MASK_CUP_INSIDE       = LIQUID | ICE;
inline constexpr uint64_t MASK_CUP_LID          = CUP_LID | CUP_SOLID;
inline constexpr uint64_t MASK_CLEANUP          = LIQUID | ICE;
inline constexpr uint64_t MASK_DROPSPACE_SENSOR = DRAGGABLE;
inline constexpr uint64_t MASK_DRAGGABLE        = DROPSPACE_SENSOR | FURNITURE | DRAGGABLE | CUP_SOLID;
inline constexpr uint64_t MASK_FURNITURE        = FURNITURE | CUP_SOLID | DRAGGABLE | ICE;
inline constexpr uint64_t MASK_ICE              = LIQUID | ICE | CUP_SOLID | CUP_INSIDE | CLEANUP | FURNITURE;
} // namespace cafe::filter
