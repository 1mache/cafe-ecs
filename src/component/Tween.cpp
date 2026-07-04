#include "Tween.h"

namespace cafe
{
void tweenTick(Tween& tween, float dt, Transform& transform)
{
    tween.elapsedTime += dt;
    // percentage of tween
    float t = std::clamp( tween.elapsedTime / tween.duration,0.f,1.f);

    auto& org  = tween.original;
    auto& targ = tween.target;
    auto lerp = [t](auto x, auto y) {return std::lerp(x,y,t);};

    // lerp every property individually
    transform =
    {
        lerp(org.x, targ.x),
        lerp(org.y, targ.y),
        lerp(org.w, targ.w),
        lerp(org.h, targ.h),
        lerp(org.rot, targ.rot)
    };
}
}