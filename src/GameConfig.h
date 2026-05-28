#pragma once

namespace cafe
{
static constexpr int   FPS            = 60;
static constexpr float FRAME_DELTA_MS = FPS ? (1000.f / FPS) : 0.f;
static constexpr float PTM            = 16.f; // pixels to meters conversion factor
static constexpr float START_WIN_W    = 1280;
static constexpr float START_WIN_H    = 720;
// static constexpr float START_CAM_X    = (START_WIN_W / (2 * PTM)) - 8.f; // cam position in world units (meters).
// static constexpr float START_CAM_Y    = (START_WIN_H / (2 * PTM)) - 5.5f;

// (basically zoom factor). not sure if needed in this game.
static constexpr float SCALE_FACTOR = 1.f;
static constexpr float GRAVITY      = 25.f;
} // namespace cafe