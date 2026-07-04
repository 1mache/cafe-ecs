#include "NapkinSystem.h"

#include "AssetManager.h"
#include "Components.h"
#include "ItemTypes.h"
#include "OrderIconFactory.h"
#include "PhysicsContext.h"
#include "SpriteSheet.h"

#include <bagel.h>
#include <box2d/box2d.h>
#include <algorithm>
#include <cmath>
#include <vector>

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
    case NapkinState::FullBlank:
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

bool isNapkinFullAtLayout(const Transform& t)
{
    const NapkinLayout full = layoutForState(NapkinState::Full);

    const float dx = t.x - full.centerX;
    const float dy = t.y - full.centerY;
    if ((dx * dx + dy * dy) > POSITION_ARRIVE_THRESHOLD_SQ)
        return false;

    if (std::fabs(t.w - full.halfW) > SIZE_SNAP_EPSILON)
        return false;

    if (std::fabs(t.h - full.halfH) > SIZE_SNAP_EPSILON)
        return false;

    return true;
}

void createCheatSheet(AssetManager& assets, bagel::Entity napkin)
{
    static constexpr auto PROPS_TEX         = "props.png";
    static constexpr auto PROPS_SPRITE_DATA = "props.json";

    const SpriteSheet& props    = assets.getSpriteSheet(PROPS_TEX, PROPS_SPRITE_DATA);
    const int          coffeeFrom = props.getTagBounds("coffee").first;

    constexpr float MARGIN = 0.05f * NAPKIN_FULL_SCREEN_W;
    constexpr float GAP    = 0.02f * NAPKIN_FULL_SCREEN_H;
    const int       n      = static_cast<int>(DrinkType::count);
    const float iconSize =
        (NAPKIN_FULL_SCREEN_H - 2.f * MARGIN - static_cast<float>(n - 1) * GAP)
        / static_cast<float>(n);

    const float x = -NAPKIN_FULL_SCREEN_W * 0.5f + MARGIN + iconSize * 0.5f;

    for (int i = 0; i < n; ++i)
    {
        const float y = -NAPKIN_FULL_SCREEN_H * 0.5f + MARGIN + iconSize * 0.5f
                      + static_cast<float>(i) * (iconSize + GAP);
        const int frame = coffeeFrom + i;

        auto icon = createOrderIcon(assets, frame, iconSize, iconSize, napkin, { x, y });
        icon.get<Drawable>().renderLayer = layer::UI4;
        icon.add(CheatSheetIcon{});
    }

    napkin.get<NapkinIntent>().state = NapkinState::Full;
}

void clearCheatSheet()
{
    static const bagel::Mask mask =
        bagel::MaskBuilder().set<CheatSheetIcon>().build();

    std::vector<bagel::ent_type> toDestroy;
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;
        toDestroy.push_back(e.entity());
    }

    for (const bagel::ent_type id : toDestroy)
        bagel::Entity(id).destroy();
}
} // namespace

void napkinSystem(AssetManager& assets, PhysicsContext& /*physics*/, float dt)
{
    static const bagel::Mask mask = bagel::MaskBuilder()
                                        .set<NapkinIntent>()
                                        .set<Transform>()
                                        .set<PhysicsBody>()
                                        .build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        auto& intent = e.get<NapkinIntent>();
        const NapkinLayout target = layoutForState(intent.state);
        auto&                  t  = e.get<Transform>();

        easeSize(t, target.halfW, target.halfH, dt);

        const b2BodyId body = e.get<PhysicsBody>().id;
        if (!b2Body_IsValid(body)) continue;

        chaseTarget(body, target.centerX, target.centerY);

        if (!isNapkinFullAtLayout(t))
            clearCheatSheet();
        else if (intent.state == NapkinState::FullBlank)
            createCheatSheet(assets, e);
    }
}
} // namespace cafe
