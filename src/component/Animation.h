#pragma once
#include <bagel.h>

#include <array>
#include <string>
#include <optional>

namespace cafe
{
constexpr float BASE_ANIM_DURATION = 0.1f;
struct AnimationFrame
{
    float      duration{BASE_ANIM_DURATION};
    int        spritesheetIndex{-1}; // index of the frame on sprite sheet. have to set
};

constexpr auto MAX_ANIM_FRAMES = 4;
struct Animation
{
    std::array<AnimationFrame, MAX_ANIM_FRAMES> frames;
    std::string                                 spriteSheetName{}; // sprite sheet to pull frames from
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