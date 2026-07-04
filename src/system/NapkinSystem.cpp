#include "NapkinSystem.h"

#include "AssetManager.h"
#include "Components.h"
#include "Glyph.h"
#include "ItemTypes.h"
#include "Menu.h"
#include "OrderIconFactory.h"
#include "PhysicsContext.h"
#include "SpriteSheet.h"

#include <bagel.h>
#include <box2d/box2d.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
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
constexpr float FULL_LAYOUT_PROXIMITY    = 0.876f;

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

    const float posTolX  = (1.f - FULL_LAYOUT_PROXIMITY) * full.halfW;
    const float posTolY  = (1.f - FULL_LAYOUT_PROXIMITY) * full.halfH;
    const float sizeTolW = (1.f - FULL_LAYOUT_PROXIMITY) * full.halfW;
    const float sizeTolH = (1.f - FULL_LAYOUT_PROXIMITY) * full.halfH;

    if (std::fabs(t.x - full.centerX) > posTolX)
        return false;

    if (std::fabs(t.y - full.centerY) > posTolY)
        return false;

    if (std::fabs(t.w - full.halfW) > sizeTolW)
        return false;

    if (std::fabs(t.h - full.halfH) > sizeTolH)
        return false;

    return true;
}

void createCheatSheet(AssetManager& assets, bagel::Entity napkin)
{
    static constexpr auto PROPS_TEX         = "props.png";
    static constexpr auto PROPS_SPRITE_DATA = "props.json";

    const SpriteSheet& props      = assets.getSpriteSheet(PROPS_TEX, PROPS_SPRITE_DATA);
    const int          coffeeFrom = props.getTagBounds("coffee").first;
    const int          menuFrom   = props.getTagBounds("menu").first;

    constexpr float MARGIN     = 0.05f * NAPKIN_FULL_SCREEN_W;
    constexpr float MARGIN_TOP = 0.07f * NAPKIN_FULL_SCREEN_H;

    constexpr int nDrinks      = static_cast<int>(DrinkType::count);
    constexpr int nIngredients = 3;
    constexpr int nRows        = 1 + nDrinks;

    const float contentW  = NAPKIN_FULL_SCREEN_W - 2.f * MARGIN;
    const float contentH  = NAPKIN_FULL_SCREEN_H - MARGIN - MARGIN_TOP;
    const float drinkColW = contentW * 0.24f;
    const float pctColW   = (contentW - drinkColW) / static_cast<float>(nIngredients);
    const float cellH     = contentH / static_cast<float>(nRows);
    const float iconSize  = std::min(std::min(drinkColW, pctColW), cellH) * 0.62f;

    const float originX = -NAPKIN_FULL_SCREEN_W * 0.5f + MARGIN;
    const float originY = -NAPKIN_FULL_SCREEN_H * 0.5f + MARGIN_TOP;

    auto colCenterX = [&](int col) -> float
    {
        if (col == 0)
            return originX + drinkColW * 0.5f;
        const int pctCol = col - 1;
        return originX + drinkColW + (static_cast<float>(pctCol) + 0.5f) * pctColW;
    };

    auto rowCenterY = [&](int row) -> float
    {
        return originY + (static_cast<float>(row) + 0.5f) * cellH;
    };

    auto cellCenter = [&](int col, int row) -> SDL_FPoint
    {
        return { colCenterX(col), rowCenterY(row) };
    };

    constexpr int   TEXT_SCALE = 1;
    // ChildOf local +Y is up; drawText anchors at glyph top - nudge up half a glyph
    // to vertically center in the cell (screen-space scenes use the opposite sign).
    constexpr float TEXT_Y_OFFSET = static_cast<float>(GLYPH_H * TEXT_SCALE) * 0.5f;

    constexpr LiquidIngredient topRowIngredients[] = {
        LiquidIngredient::Coffee,
        LiquidIngredient::Milk,
        LiquidIngredient::Water,
    };

    for (int j = 0; j < nIngredients; ++j)
    {
        const SDL_FPoint pos   = cellCenter(j + 1, nDrinks);
        const int        frame = menuFrom + j;

        auto icon = createOrderIcon(assets, frame, iconSize, iconSize, napkin, pos);
        icon.get<Drawable>().renderLayer = layer::UI4;
        icon.add(CheatSheetIcon{});
    }

    for (int i = 0; i < nDrinks; ++i)
    {
        const SDL_FPoint pos   = cellCenter(0, i);
        const int        frame = coffeeFrom + i;

        auto icon = createOrderIcon(assets, frame, iconSize, iconSize, napkin, pos);
        icon.get<Drawable>().renderLayer = layer::UI4;
        icon.add(CheatSheetIcon{});
    }

    for (int i = 0; i < nDrinks; ++i)
    {
        const DrinkRecipe& recipe = recipeFor(static_cast<DrinkType>(i));

        for (int j = 0; j < nIngredients; ++j)
        {
            const LiquidIngredient ing = topRowIngredients[j];
            const int pct = static_cast<int>(
                std::lround(recipe.ratio[static_cast<size_t>(ing)] * 100.f));

            char buf[8];
            std::snprintf(buf, sizeof(buf), "%dx", pct);

            const SDL_FPoint center = cellCenter(j + 1, i);
            const SDL_FPoint pos    = { center.x, center.y + TEXT_Y_OFFSET };
            bagel::Entity::create().addAll(
                Transform{},
                TextLabel{ buf, TEXT_SCALE, TextAlign::Center },
                ChildOf{ napkin, pos },
                CheatSheetIcon{});
        }
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
