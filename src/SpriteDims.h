#pragma once
#include "SDL3/SDL_rect.h"

namespace cafe
{
static constexpr SDL_FPoint CUP_DIMS      = {24, 24};
static constexpr SDL_FPoint MINI_CUP_DIMS = {16, 16};
static constexpr SDL_FPoint PROP_DIMS     = {16, 16};
static constexpr SDL_FPoint PERSON_DIMS   = {32, 48};
static constexpr SDL_FPoint BUBBLE_DIMS   = {64, 48};
static constexpr float      BUBBLE_SCALE  = 1.f / 1.7f;
// Max icons that stack vertically in one bubble column (up to 3 beverages /
// pastries per order). The bubble height is sized so this many rows fit.
static constexpr int        BUBBLE_MAX_ICONS_PER_COLUMN = 3;
} // namespace cafe