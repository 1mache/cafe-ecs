#pragma once

#include "GameConfig.h"
#include "Transform.h"

#include <bagel.h>

enum class NapkinState
{
    Hidden,
    Toggle,
    FullBlank,
    Full
};

namespace cafe
{
// Texture layout — must match res/napkin.png pixel size.
constexpr float NAPKIN_HIDDEN_TEX_W = 100.f;
constexpr float NAPKIN_HIDDEN_TEX_H = 90.f;
constexpr float NAPKIN_HIDDEN_SCALE = 0.25f;

constexpr float NAPKIN_HIDDEN_HALF_W =
    texToWorldScale(NAPKIN_HIDDEN_TEX_W) * NAPKIN_HIDDEN_SCALE;
constexpr float NAPKIN_HIDDEN_HALF_H =
    texToWorldScale(NAPKIN_HIDDEN_TEX_H) * NAPKIN_HIDDEN_SCALE;

// Hidden-state layout — top quarter visible at bottom-center.
constexpr float NAPKIN_HIDDEN_CENTER_X = 0.f;
constexpr float NAPKIN_HIDDEN_CENTER_Y =
    -texToWorldDistance(LOGICAL_H / 2.f) - NAPKIN_HIDDEN_HALF_H / 2.f;

// Hidden-state screen-space hitbox (camera at origin).
constexpr float NAPKIN_HIDDEN_HITBOX_X =
    LOGICAL_W * 0.5f + (NAPKIN_HIDDEN_CENTER_X - NAPKIN_HIDDEN_HALF_W) * PTM * SCALE_FACTOR;
constexpr float NAPKIN_HIDDEN_HITBOX_Y =
    LOGICAL_H * 0.5f - (NAPKIN_HIDDEN_CENTER_Y + NAPKIN_HIDDEN_HALF_H) * PTM * SCALE_FACTOR;
constexpr float NAPKIN_HIDDEN_HITBOX_W = worldToScreenScale(NAPKIN_HIDDEN_HALF_W);
constexpr float NAPKIN_HIDDEN_HITBOX_H = worldToScreenScale(NAPKIN_HIDDEN_HALF_H);

// Toggle-state layout — same size as hidden, former hidden location (top half peek).
constexpr float NAPKIN_TOGGLE_CENTER_X = 0.f;
constexpr float NAPKIN_TOGGLE_CENTER_Y = -texToWorldDistance(LOGICAL_H / 2.f);
constexpr float NAPKIN_TOGGLE_HALF_W = NAPKIN_HIDDEN_HALF_W;
constexpr float NAPKIN_TOGGLE_HALF_H = NAPKIN_HIDDEN_HALF_H;

// Toggle-state screen-space hitbox (camera at origin).
constexpr float NAPKIN_TOGGLE_HITBOX_X =
    LOGICAL_W * 0.5f + (NAPKIN_TOGGLE_CENTER_X - NAPKIN_TOGGLE_HALF_W) * PTM * SCALE_FACTOR;
constexpr float NAPKIN_TOGGLE_HITBOX_Y =
    LOGICAL_H * 0.5f - (NAPKIN_TOGGLE_CENTER_Y + NAPKIN_TOGGLE_HALF_H) * PTM * SCALE_FACTOR;
constexpr float NAPKIN_TOGGLE_HITBOX_W = worldToScreenScale(NAPKIN_TOGGLE_HALF_W);
constexpr float NAPKIN_TOGGLE_HITBOX_H = worldToScreenScale(NAPKIN_TOGGLE_HALF_H);

// Full-state layout — centered, aspect-fit inside 80% of screen.
constexpr float NAPKIN_FULL_MAX_SCREEN_W = LOGICAL_W * 0.8f;
constexpr float NAPKIN_FULL_MAX_SCREEN_H = LOGICAL_H * 0.8f;
constexpr float NAPKIN_FULL_SCREEN_H = NAPKIN_FULL_MAX_SCREEN_H;
constexpr float NAPKIN_FULL_SCREEN_W =
    NAPKIN_FULL_SCREEN_H * (NAPKIN_HIDDEN_TEX_W / NAPKIN_HIDDEN_TEX_H);
constexpr float NAPKIN_FULL_CENTER_X = 0.f;
constexpr float NAPKIN_FULL_CENTER_Y = 0.f;
constexpr float NAPKIN_FULL_HALF_W = screenToWorldScale(NAPKIN_FULL_SCREEN_W);
constexpr float NAPKIN_FULL_HALF_H = screenToWorldScale(NAPKIN_FULL_SCREEN_H);

// Full-state screen-space hitbox (camera at origin).
constexpr float NAPKIN_FULL_HITBOX_X =
    LOGICAL_W * 0.5f + (NAPKIN_FULL_CENTER_X - NAPKIN_FULL_HALF_W) * PTM * SCALE_FACTOR;
constexpr float NAPKIN_FULL_HITBOX_Y =
    LOGICAL_H * 0.5f - (NAPKIN_FULL_CENTER_Y + NAPKIN_FULL_HALF_H) * PTM * SCALE_FACTOR;
constexpr float NAPKIN_FULL_HITBOX_W = worldToScreenScale(NAPKIN_FULL_HALF_W);
constexpr float NAPKIN_FULL_HITBOX_H = worldToScreenScale(NAPKIN_FULL_HALF_H);

struct NapkinIntent
{
    NapkinState state{ NapkinState::Hidden };
};
} // namespace cafe

template <> struct bagel::Storage<cafe::NapkinIntent> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::NapkinIntent>;
};
