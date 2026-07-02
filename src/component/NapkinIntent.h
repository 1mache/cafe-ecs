#pragma once

#include "GameConfig.h"
#include "Transform.h"

#include <bagel.h>

enum class NapkinState
{
    Hidden,
    Toggle,
    Full
};

namespace cafe
{
// Hidden-state layout — must match res/napkin.png pixel size.
constexpr float NAPKIN_HIDDEN_TEX_W = 100.f;
constexpr float NAPKIN_HIDDEN_TEX_H = 90.f;
constexpr float NAPKIN_HIDDEN_SCALE = 0.25f;

constexpr float NAPKIN_HIDDEN_HALF_W =
    texToWorldScale(NAPKIN_HIDDEN_TEX_W) * NAPKIN_HIDDEN_SCALE;
constexpr float NAPKIN_HIDDEN_HALF_H =
    texToWorldScale(NAPKIN_HIDDEN_TEX_H) * NAPKIN_HIDDEN_SCALE;
constexpr float NAPKIN_HIDDEN_CENTER_X = 0.f;
constexpr float NAPKIN_HIDDEN_CENTER_Y = -texToWorldDistance(LOGICAL_H / 2.f);

// Hidden-state screen-space hitbox (camera at origin).
constexpr float NAPKIN_HIDDEN_HITBOX_X =
    LOGICAL_W * 0.5f + (NAPKIN_HIDDEN_CENTER_X - NAPKIN_HIDDEN_HALF_W) * PTM * SCALE_FACTOR;
constexpr float NAPKIN_HIDDEN_HITBOX_Y =
    LOGICAL_H * 0.5f - (NAPKIN_HIDDEN_CENTER_Y + NAPKIN_HIDDEN_HALF_H) * PTM * SCALE_FACTOR;
constexpr float NAPKIN_HIDDEN_HITBOX_W = worldToScreenScale(NAPKIN_HIDDEN_HALF_W);
constexpr float NAPKIN_HIDDEN_HITBOX_H = worldToScreenScale(NAPKIN_HIDDEN_HALF_H);

struct NapkinIntent
{
    NapkinState state{ NapkinState::Hidden };
};
} // namespace cafe

template <> struct bagel::Storage<cafe::NapkinIntent> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::NapkinIntent>;
};
