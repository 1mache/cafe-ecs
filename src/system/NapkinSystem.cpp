#include "NapkinSystem.h"

#include "Components.h"
#include "PhysicsContext.h"

#include <bagel.h>
#include <box2d/box2d.h>
#include <algorithm>
#include <cmath>

namespace cafe
{
namespace
{
constexpr float MAX_FOLLOW_SPEED         = 40.f;
constexpr float FOLLOW_SPEED_GAIN        = 32.f;
constexpr float POSITION_ARRIVE_THRESHOLD  = 0.01f;
constexpr float POSITION_ARRIVE_THRESHOLD_SQ =
    POSITION_ARRIVE_THRESHOLD * POSITION_ARRIVE_THRESHOLD;
constexpr float SIZE_LERP_RATE           = 10.f;
constexpr float SIZE_SNAP_EPSILON        = 0.001f;

struct NapkinLayout
{
    float centerX{};
    float centerY{};
    float halfW{};
    float halfH{};
};

NapkinLayout layoutForState(NapkinState state)
{
    switch (state)
    {
    case NapkinState::Hidden:
        return NapkinLayout{
            .centerX = NAPKIN_HIDDEN_CENTER_X,
            .centerY = NAPKIN_HIDDEN_CENTER_Y,
            .halfW   = NAPKIN_HIDDEN_HALF_W,
            .halfH   = NAPKIN_HIDDEN_HALF_H,
        };
    case NapkinState::Toggle:
        return NapkinLayout{
            .centerX = NAPKIN_TOGGLE_CENTER_X,
            .centerY = NAPKIN_TOGGLE_CENTER_Y,
            .halfW   = NAPKIN_TOGGLE_HALF_W,
            .halfH   = NAPKIN_TOGGLE_HALF_H,
        };
    case NapkinState::Full:
        return NapkinLayout{
            .centerX = NAPKIN_FULL_CENTER_X,
            .centerY = NAPKIN_FULL_CENTER_Y,
            .halfW   = NAPKIN_FULL_HALF_W,
            .halfH   = NAPKIN_FULL_HALF_H,
        };
    }
    return layoutForState(NapkinState::Hidden);
}

float speedByDist(float dist)
{
    return std::min(dist * FOLLOW_SPEED_GAIN, MAX_FOLLOW_SPEED);
}

void chaseTarget(b2BodyId body, float targetX, float targetY)
{
    b2Body_SetGravityScale(body, 0.f);

    const b2Vec2 current = b2Body_GetPosition(body);
    const b2Vec2 delta   = b2Sub({ targetX, targetY }, current);

    if (b2LengthSquared(delta) <= POSITION_ARRIVE_THRESHOLD_SQ)
    {
        b2Body_SetLinearVelocity(body, { 0.f, 0.f });
        return;
    }

    const float  dist  = b2Length(delta);
    const float  speed = speedByDist(dist);
    const b2Vec2 dir   = b2Normalize(delta);
    b2Body_SetLinearVelocity(body, b2MulSV(speed, dir));
}

void easeSize(Transform& t, float targetHalfW, float targetHalfH, float dt)
{
    const float lerpFactor = std::min(1.f, SIZE_LERP_RATE * dt);

    const float dw = targetHalfW - t.w;
    if (std::fabs(dw) <= SIZE_SNAP_EPSILON)
        t.w = targetHalfW;
    else
        t.w += dw * lerpFactor;

    const float dh = targetHalfH - t.h;
    if (std::fabs(dh) <= SIZE_SNAP_EPSILON)
        t.h = targetHalfH;
    else
        t.h += dh * lerpFactor;
}
} // namespace

void napkinSystem(PhysicsContext& /*physics*/, float dt)
{
    static const bagel::Mask mask = bagel::MaskBuilder()
                                        .set<NapkinIntent>()
                                        .set<Transform>()
                                        .set<PhysicsBody>()
                                        .build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        const NapkinLayout target = layoutForState(e.get<NapkinIntent>().state);
        auto&                  t  = e.get<Transform>();

        easeSize(t, target.halfW, target.halfH, dt);

        const b2BodyId body = e.get<PhysicsBody>().id;
        if (!b2Body_IsValid(body)) continue;

        chaseTarget(body, target.centerX, target.centerY);
    }
}
} // namespace cafe
