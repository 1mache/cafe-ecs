#pragma once
#include "Transform.h"

#include <array>
#include <string>
#include <optional>

namespace cafe
{
constexpr float BASE_ANIM_DURATION = 0.1f;
struct AnimationFrame
{
    Transform  relativeTransform{0.f, 0.f,1.f, 1.f, 0.f}; // animated objects transform = original + this transform
    float      duration{BASE_ANIM_DURATION};
    int        spritesheetIndex{-1}; // index of the frame on sprite sheet. have to set
};

constexpr auto MAX_ANIM_FRAMES = 4;
struct Animation
{
    std::array<AnimationFrame, MAX_ANIM_FRAMES> frames;
    std::string                                 spriteSheetName{}; // empty => not a sprite sheet animation
    bool hasSpriteSheet() const                 { return spriteSheetName.size() > 0; }
    int                                         currentFrame{0};
    float                                       timer{};
    int                                         frameCount{1};
    bool                                        isLoop{false};
    bool                                        ended{false}; // force end through this
};

std::optional<int> getNextAnimationFrame(Animation& animation);
void updateAnimationWithFrame(Animation& animation, int frameId);
}

template <> struct bagel::Storage<cafe::Animation> final : NoInstance {using type = SparseStorage<cafe::Animation>; };