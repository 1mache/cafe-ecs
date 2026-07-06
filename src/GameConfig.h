#pragma once

namespace cafe
{
static constexpr auto  WINDOW_TITLE   = "Entity Coffee System";
static constexpr int   FPS            = 60;
static constexpr float FRAME_DELTA_MS = FPS ? (1000.f / FPS) : 0.f;
static constexpr float FADE_DURATION  = 0.3f; // seconds, per fade in/out
static constexpr float PTM            = 8.f;  // pixels to meters conversion factor
static constexpr float START_WIN_W    = 1280; // physical window size
static constexpr float START_WIN_H    = 720;
// SDL upscales this logical canvas to the physical window automatically.
static constexpr int LOGICAL_W = 160;
static constexpr int LOGICAL_H = 90;
// static constexpr float START_CAM_X    = (START_WIN_W / (2 * PTM)) - 8.f; // cam position in world units (meters).
// static constexpr float START_CAM_Y    = (START_WIN_H / (2 * PTM)) - 5.5f;

// Pure zoom inside logical space. Display-fit handled by SDL, not here.
static constexpr float SCALE_FACTOR = 1.f;
static constexpr float GRAVITY      = 35.f;
} // namespace cafe