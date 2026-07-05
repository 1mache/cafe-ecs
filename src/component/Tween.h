#pragma once
#include "Transform.h"

namespace cafe
{
struct Tween
{
    enum Kind
    {
        Smooth, Exponential, Spring, count
    };

    Transform original;
    Transform target;
    Kind kind{Smooth};
    float duration;
    float elapsedTime{0.f};
};

// 1 tick of the tween. nudges the transform to the target based on dt
void tweenTick(Tween& tween, float dt, Transform& transform);
// progress in range [0.f, 1.f]
float getTweenProgress(const Tween& tween);
}

template<> struct bagel::Storage<cafe::Tween> final : NoInstance {using type = PackedStorage<cafe::Tween>;};