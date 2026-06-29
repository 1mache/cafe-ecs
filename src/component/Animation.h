#pragma once
#include "OrderGrade.h"
#include "Transform.h"

#include <array>
#include <string>
#include <optional>

namespace cafe
{
constexpr float BASE_ANIM_DURATION = 0.1f;
struct AnimationFrame
{
    Transform transform{}; // does the transform of the body change
    float     duration{BASE_ANIM_DURATION};
    int       spriteIndex{-1}; // index of the frame on sprite sheet. have to set
};

constexpr auto MAX_ANIM_FRAMES = 4;
struct Animation
{
    std::array<AnimationFrame, MAX_ANIM_FRAMES> frames;
    int                                         currentFrame{0};
    int                                         frameCount{1};
    std::string                                 assetName{};
    bool                                        isSpriteSheet{false};
    bool                                        isLoop{false};
    bool                                        ended{false}; // force end through this
};

std::optional<int> getNextAnimationFrame(Animation& animation);
}

template <> struct bagel::Storage<cafe::Animation> final : NoInstance {using type = SparseStorage<cafe::Animation>; };