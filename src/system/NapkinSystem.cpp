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
constexpr float FULL_LAYOUT_PROXIMITY    = 0.3f;
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

void createCheatSheet(AssetManager& assets, bagel::Entity napkin)
{
    static constexpr auto PROPS_TEX         = "props.png";
    static constexpr auto PROPS_SPRITE_DATA = "props.json";

    const SpriteSheet& props      = assets.getSpriteSheet(PROPS_TEX, PROPS_SPRITE_DATA);
    const int          coffeeFrom = props.getTagBounds("coffee").first;
    const int          menuFrom   = props.getTagBounds("menu").first;

    constexpr float MARGIN     = 0.05f * NAPKIN_FULL_SCREEN_W;
    constexpr float MARGIN_TOP = 0.07f * NAPKIN_FULL_SCREEN_H;

    constexpr int nDrinks         = static_cast<int>(DrinkType::count);
    constexpr int nIngredientCols = 3;
    constexpr int nDataCols       = nIngredientCols + 1; // + fill column
    constexpr int nRows           = 1 + nDrinks;

    const float contentW  = NAPKIN_FULL_SCREEN_W - 2.f * MARGIN;
    const float contentH  = NAPKIN_FULL_SCREEN_H - MARGIN - MARGIN_TOP;
    const float drinkColW = contentW * 0.20f;
    const float pctColW   = (contentW - drinkColW) / static_cast<float>(nDataCols);
    const float cellH     = contentH / static_cast<float>(nRows);
    const float iconSize  = std::min(std::min(drinkColW, pctColW), cellH) * 0.58f;

    // Scale text to fit the widest label ("100%") inside a data column.
    const float textScale = std::clamp(
        pctColW * 0.85f / textWidth("100%", 1.f), 0.45f, 1.f);
    const float textYOffset = static_cast<float>(GLYPH_H) * textScale * 0.5f;

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

    constexpr LiquidIngredient topRowIngredients[] = {
        LiquidIngredient::Coffee,
        LiquidIngredient::Milk,
        LiquidIngredient::Water,
    };

    for (int j = 0; j < nIngredientCols; ++j)
    {
        const SDL_FPoint pos   = cellCenter(j + 1, nDrinks);
        const int        frame = menuFrom + j;

        auto icon = createOrderIcon(assets, frame, iconSize, iconSize, napkin, pos);
        icon.get<Drawable>().renderLayer = layer::UI4;
        icon.add(CheatSheetIcon{});
    }

    {
        const SDL_FPoint pos   = cellCenter(nIngredientCols + 1, nDrinks);
        const int        frame = menuFrom + 3; // fillAmount (menu frame 18)

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

        for (int j = 0; j < nIngredientCols; ++j)
        {
            const LiquidIngredient ing = topRowIngredients[j];
            const int pct = static_cast<int>(
                std::lround(recipe.ratio[static_cast<size_t>(ing)] * 100.f));

            char buf[8];
            std::snprintf(buf, sizeof(buf), "%d%%", pct);

            const SDL_FPoint center = cellCenter(j + 1, i);
            const SDL_FPoint pos    = { center.x, center.y + textYOffset };
            bagel::Entity::create().addAll(
                Transform{},
                TextLabel{ buf, textScale, TextAlign::Center },
                ChildOf{ napkin, pos },
                CheatSheetIcon{});
        }

        {
            const int fillPct = static_cast<int>(std::lround(recipe.targetFill * 100.f));

            char buf[8];
            std::snprintf(buf, sizeof(buf), "%d%%", fillPct);

            const SDL_FPoint center = cellCenter(nIngredientCols + 1, i);
            const SDL_FPoint pos    = { center.x, center.y + textYOffset };
            bagel::Entity::create().addAll(
                Transform{},
                TextLabel{ buf, textScale, TextAlign::Center },
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
    static const Transform fullXform = transformForState(NapkinState::Full);

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
                             .kind = Tween::Spring, .duration = NAPKIN_TWEEN_DURATION };
            if (e.has<Tween>())
                e.get<Tween>() = tw;
            else
                e.add(tw);
        }

        // heading to (or resting at) full layout, far enough along to reveal/hide the cheat
        // sheet. Uses the tween's linear progress, not the eased transform: Spring's
        // easeOutElastic overshoots and wobbles past the target, so a transform-distance check
        // crosses the threshold multiple times instead of once, clearing the sheet right after
        // it was created.
        const bool atFullLayout = e.has<Tween>()
            ? sameTarget(e.get<Tween>().target, fullXform)
                  && getTweenProgress(e.get<Tween>()) >= FULL_LAYOUT_PROXIMITY
            : sameTarget(t, fullXform);

        if (!atFullLayout)
            clearCheatSheet();
        else if (intent.state == NapkinState::FullBlank)
            createCheatSheet(assets, e);
    }
}
} // namespace cafe
