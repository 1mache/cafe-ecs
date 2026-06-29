#include "Animation.h"

#include "Utils.h"

namespace cafe
{
std::optional<int> getNextAnimationFrame(Animation& animation)
{
    if (animation.ended) return std::nullopt;

    animation.currentFrame++;
    if (animation.currentFrame >= animation.frameCount)
    {
        animation.ended = !animation.isLoop; // isLoop -> doesnt end
        animation.currentFrame = 0;
    }

    if (animation.ended)
        return std::nullopt;

    return animation.currentFrame;
}
void updateAnimationWithFrame(Animation& animation, int frameId)
{
    assertFatal(frameId < animation.frameCount,
        "Animation frame index out of range " + std::to_string(frameId));
    AnimationFrame& frame = animation.frames[toSizet(frameId)];
    animation.currentFrame = frameId;
    animation.timer = frame.duration;
}
} // namespace cafe