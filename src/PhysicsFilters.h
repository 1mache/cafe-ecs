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
inline constexpr uint64_t LIQUID      = 1ull << 0; // coffee drops
inline constexpr uint64_t CUP_SOLID   = 1ull << 1; // cup walls & bottom
inline constexpr uint64_t CUP_INSIDE  = 1ull << 2; // cup interior sensor
inline constexpr uint64_t CLEANUP     = 1ull << 3; // out-of-bounds sensor

inline constexpr uint64_t MASK_LIQUID     = CUP_SOLID | CUP_INSIDE | CLEANUP;
inline constexpr uint64_t MASK_CUP_SOLID  = LIQUID;
inline constexpr uint64_t MASK_CUP_INSIDE = LIQUID;
inline constexpr uint64_t MASK_CLEANUP    = LIQUID;
} // namespace cafe::filter
