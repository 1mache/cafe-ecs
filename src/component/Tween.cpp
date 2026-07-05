#include "Tween.h"

#include <cmath>
#include <numbers>

namespace cafe
{
namespace
{
// exponential ease-out sharpness: bigger = snappier settle
constexpr float EXP_SHARPNESS = 10.f;
// easeOutElastic oscillation period constant (2π/3)
constexpr float ELASTIC_C = 2.f * std::numbers::pi_v<float> / 3.f;

// remaps linear progress t into an eased [0,1] value per tween kind
float easeProgress(Tween::Kind kind, float t)
{
    switch (kind)
    {
    case Tween::Exponential:
        // smooth ease-out: fast start, asymptotic settle, no overshoot
        if (t >= 1.f) return 1.f; // land exactly on target (exp2 never hits 1 itself)
        return 1.f - std::exp2(-EXP_SHARPNESS * t);
    case Tween::Spring:
        // easeOutElastic: overshoots target then wobbles in (bouncy)
        if (t <= 0.f) return 0.f;
        if (t >= 1.f) return 1.f;
        return std::pow(2.f, -12.f * t) * std::sin((t * 8.f - 0.75f) * ELASTIC_C) + 1.f;
    case Tween::Smooth:
    default:
        // linear
        return t;
    }
}
} // namespace

void tweenTick(Tween& tween, float dt, Transform& transform)
{
    tween.elapsedTime += dt;
    // linear percentage of tween, then eased by kind
    float t = getTweenProgress(tween);
    float e = easeProgress(tween.kind, t);

    auto& org  = tween.original;
    auto& targ = tween.target;
    auto lerp = [e](auto a, auto b) {return std::lerp(a, b, e);};

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
float getTweenProgress(const Tween& tween)
{
    // percentage of tween
    return std::clamp( tween.elapsedTime / tween.duration,0.f,1.f);
}
} // namespace cafe