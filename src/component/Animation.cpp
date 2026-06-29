#include "Animation.h"

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
}