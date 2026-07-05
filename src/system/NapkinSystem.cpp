#include "NapkinSystem.h"

#include "AssetManager.h"
#include "Components.h"
#include "Glyph.h"
#include "ItemTypes.h"
#include "Menu.h"
#include "OrderIconFactory.h"
#include "SpriteSheet.h"
#include "Transform.h"
#include "Tween.h"

#include <bagel.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <vector>

namespace cafe
{
namespace
{
constexpr float FULL_LAYOUT_PROXIMITY    = 0.876f;
// duration of every napkin Hidden/Toggle/Full leg, tune by feel
constexpr float NAPKIN_TWEEN_DURATION    = 0.5f;

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

Transform transformForState(NapkinState s)
{
    const NapkinLayout l = layoutForState(s);
    return Transform{ .x = l.centerX, .y = l.centerY, .w = l.halfW, .h = l.halfH };
}

// exact compare is safe: animating branch compares two constexpr-derived targets;
// resting branch relies on the Exponential ease landing exactly on 1.0 at t=1 (see
// Tween::easeProgress), so a completed tween leaves Transform bit-exact == target.
bool sameTarget(const Transform& a, const Transform& b)
{
    return a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h;
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

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;
        e.addAll(Destroy{});
    }
}
} // namespace

void napkinSystem(AssetManager& assets, float /*dt*/)
{
    static const bagel::Mask mask = bagel::MaskBuilder()
                                        .set<NapkinIntent>()
                                        .set<Transform>()
                                        .build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;

        auto&           intent  = e.get<NapkinIntent>();
        auto&           t       = e.get<Transform>();
        const Transform desired = transformForState(intent.state);

        // already heading there? (in-flight tween's target, or resting transform)
        const bool onTarget = e.has<Tween>()
            ? sameTarget(e.get<Tween>().target, desired)
            : sameTarget(t, desired);

        if (!onTarget) // state changed: (re)start/retarget from current transform
        {
            const Tween tw{ .original = t, .target = desired,
                             .kind = Tween::Exponential, .duration = NAPKIN_TWEEN_DURATION };
            if (e.has<Tween>())
                e.get<Tween>() = tw;
            else
                e.add(tw);
        }

        if (!isNapkinFullAtLayout(t))
            clearCheatSheet();
        else if (intent.state == NapkinState::FullBlank)
            createCheatSheet(assets, e);
    }
}
} // namespace cafe
