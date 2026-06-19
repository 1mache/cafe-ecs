#pragma once
#include <cstdint>

// Box2D collision categories shared across the project.
//
// Bit assignments are deliberately small so teammates can claim the next
// free bits without coordinating in source.
//
//   bits 0-3  : owned by the fluid/cup feature (this file)
//   bits 4-7  : reserved for drag-and-drop (DROPSPACE_SENSOR, DRAGGABLE, ...)
//   bits 8+   : unassigned. Coordinate before claiming.
//
// Each mask only matches bits this feature controls, so a teammate's
// fixture cannot accidentally interact with ours even if their mask is wide.

namespace cafe::filter
{
inline constexpr uint64_t LIQUID           = 1ull << 0; // coffee drops
inline constexpr uint64_t CUP_SOLID        = 1ull << 1; // cup walls & bottom
inline constexpr uint64_t CUP_INSIDE       = 1ull << 2; // cup interior sensor
inline constexpr uint64_t CLEANUP          = 1ull << 3; // out-of-bounds sensor
inline constexpr uint64_t DROPSPACE_SENSOR = 1ull << 4; // drop-zone sensor
inline constexpr uint64_t DRAGGABLE        = 1ull << 5; // draggable visitor shape
inline constexpr uint64_t FURNITURE        = 1ull << 6; // solid furniture
inline constexpr uint64_t ICE              = 1ull << 7; // ice cube: rests on furniture, caught by cup

inline constexpr uint64_t MASK_LIQUID           = CUP_SOLID | CUP_INSIDE | CLEANUP;
inline constexpr uint64_t MASK_CUP_SOLID        = LIQUID | FURNITURE | CUP_SOLID | DRAGGABLE | ICE;
inline constexpr uint64_t MASK_CUP_INSIDE       = LIQUID | ICE;
inline constexpr uint64_t MASK_CLEANUP          = LIQUID | ICE;
inline constexpr uint64_t MASK_DROPSPACE_SENSOR = DRAGGABLE;
inline constexpr uint64_t MASK_DRAGGABLE        = DROPSPACE_SENSOR | FURNITURE | DRAGGABLE | CUP_SOLID;
inline constexpr uint64_t MASK_FURNITURE        = FURNITURE | CUP_SOLID | DRAGGABLE | ICE;
// Ice rests on furniture (the counter), is contained by cup walls, is caught by
// the cup interior sensor, and is destroyed by cleanup if it misses. It does NOT
// include LIQUID, so it stays out of liquid-vs-liquid behavior.
inline constexpr uint64_t MASK_ICE              = CUP_SOLID | CUP_INSIDE | CLEANUP | FURNITURE;
} // namespace cafe::filter
