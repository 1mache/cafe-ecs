#include "DayReportScene.h"

#include "Components.h"        // ShopButton, NextDayButton, Transform, Drawable
#include "DayReport.h"
#include "DayState.h"
#include "Entities.h"          // destroyAllGameEntities
#include "GameConfig.h"
#include "Glyph.h"             // textWidth, GLYPH_H
#include "RenderContext.h"
#include "RenderLayers.h"
#include "RenderSystem.h"      // drawSystem
#include "ShopSystem.h"
#include "Text.h"              // drawText
#include "Texture.h"
#include "Transform.h"         // transformToFrect
#include "UpgradeCatalog.h"
#include "UpgradeState.h"
#include "entity/CafeEnvironmentFactory.h" // createBg
#include <SDL3/SDL.h>
#include <bagel.h>
#include <string>

namespace cafe
{
namespace
{
constexpr auto  FONT_TEX = "font.png";
constexpr auto  BG_TEX   = "day_report_screen.png"; // framed "TV" backdrop; content fits its black screen area
constexpr int   SCALE    = 1;

// --- Stats block (immediate-mode overlay). The backdrop's black screen area is
// x:[5,154] y:[5,75] on the 160x90 logical canvas; these keep text inside it. ---
constexpr float STAT_TOP_Y    = 8.f;
constexpr float STAT_LINE_H   = 9.f;
constexpr float STAT_MARGIN_X = 8.f;

// --- Upgrade bars in WORLD units (camera at origin: 1 unit = 8 px, center = 80,45).
// Sized/positioned to stay inside the backdrop's black screen area (see above)
// and clear of its bottom bezel (screen y >= 76). ---
constexpr float BTN_HALF_W  = 8.75f;   // upgrade bars: ~140 px wide, inset from the frame
constexpr float BTN_HALF_H  = 0.5625f; // ~9 px tall (one text line + padding)
constexpr float BTN_X       = 0.f;     // centered horizontally
constexpr float FIRST_BTN_Y = -1.5625f; // first upgrade bar (~screen y 57.5)
constexpr float BTN_GAP_Y   = 1.375f;   // world gap between stacked bars

// The backdrop already draws a red "power" button in its bottom-right bezel
// (screen px center ~149,84, ~9 px across); this is just its click hit-box —
// no Drawable, so nothing is rendered over the baked-in art.
constexpr WorldPos NEXT_DAY_HITBOX_POS  = { 8.625f, -4.875f };
constexpr float    NEXT_DAY_HITBOX_HALF = 0.625f; // ~5 px half-size, a bit larger than the icon

constexpr float LABEL_PAD      = 4.f;                                  // text inset from bar edge
constexpr float LABEL_Y_OFFSET = -static_cast<float>(GLYPH_H) * 0.5f;  // vertical centring

// State colours for the upgrade bar fill (see AskUserQuestion: "colour by state").
constexpr SDL_Color COLOR_AFFORD = { 110, 190,  70, 255 }; // can buy      -> green
constexpr SDL_Color COLOR_POOR   = {  78,  78,  86, 255 }; // too expensive-> gray
constexpr SDL_Color COLOR_MAXED  = { 214, 172,  58, 255 }; // maxed out    -> gold

bagel::Entity makeButtonEntity(WorldPos pos, float halfW, float halfH)
{
    auto e = bagel::Entity::create();
    e.addAll(
        Transform{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH },
        // Null texture => drawSystem fills the rect with tint (set per frame below).
        Drawable{ nullptr, SDL_FRect{}, layer::UI1 }
    );
    return e;
}

// Right-hand status for an upgrade row, e.g. "LV0 50" or "LV3 MAX".
std::string shopStatus(UpgradeId id, int lvl)
{
    std::string s = "LV" + std::to_string(lvl);
    s += isMaxed(id, lvl) ? " MAX" : (" " + std::to_string(upgradeCost(id, lvl)));
    return s;
}
} // namespace

void DayReportScene::onInit()
{
    getAssetManager().getTexture(FONT_TEX); // warm the cache
    createBg(getAssetManager(), BG_TEX);     // framed backdrop, replaces the flat clear colour

    // Bank this day's score into the persistent wallet. onInit runs exactly once
    // per scene instance (CafeGame makes a fresh DayReportScene each day), so this
    // adds the finished day's score to money exactly once.
    UpgradeState::bankScore(DayState::score());

    // One clickable bar per upgrade (world-space entities, drawn as filled bars).
    for (int i = 0; i < static_cast<int>(UpgradeId::count); ++i)
    {
        auto e = makeButtonEntity({ BTN_X, FIRST_BTN_Y - static_cast<float>(i) * BTN_GAP_Y },
                                  BTN_HALF_W, BTN_HALF_H);
        e.add(ShopButton{ .id = static_cast<UpgradeId>(i) });
    }

    // Next-day trigger: a Transform-only hit-box over the backdrop's baked-in power
    // button. No Drawable, so shopInputSystem still hit-tests it (Transform-only
    // mask, see NextDayButton.h) but drawSystem has nothing to draw for it.
    auto next = bagel::Entity::create();
    next.addAll(
        Transform{ .x = NEXT_DAY_HITBOX_POS.x, .y = NEXT_DAY_HITBOX_POS.y,
                   .w = NEXT_DAY_HITBOX_HALF, .h = NEXT_DAY_HITBOX_HALF },
        NextDayButton{}
    );
}

bool DayReportScene::onUpdate(float /*dt*/)
{
    SDL_Renderer* renderer = getRenderer();
    auto&         assets   = getAssetManager();

    bool nextDay = false, exitRequested = false;
    shopInputSystem(renderer, nextDay, exitRequested);
    shopPurchaseSystem();

    if (exitRequested) { requestNext(SceneId::Quit);     return false; }
    if (nextDay)       { requestNext(SceneId::MainGame); return false; }

    static const bagel::Mask shopMask =
        bagel::MaskBuilder().set<ShopButton>().set<Transform>().set<Drawable>().build();

    // Colour each upgrade bar by its purchase state, then let drawSystem fill it.
    const int money = UpgradeState::money();
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(shopMask)) continue;
        const ShopButton& b   = e.get<ShopButton>();
        const int         lvl = UpgradeState::level(b.id);
        e.get<Drawable>().tint = isMaxed(b.id, lvl) ? COLOR_MAXED
                               : (money >= upgradeCost(b.id, lvl) ? COLOR_AFFORD : COLOR_POOR);
    }

    SDL_RenderClear(renderer);

    drawSystem(renderer); // backdrop + upgrade bars (world-space, engine render path)

    const Texture&  font = assets.getTexture(FONT_TEX);
    const WorldPos  cam  = RenderContext::getCameraPos();

    // --- Stats block: DAY/SERVED/LOST/SCORE + MONEY (skip spacers + the obsolete
    // "CLICK TO CONTINUE" footer; next-day now happens via the power button). ---
    float y = STAT_TOP_Y;
    for (const auto& row : buildReportRows())
    {
        if (row.label.empty() || row.label == "CLICK TO CONTINUE")
            continue;
        drawText(renderer, font, row.label, STAT_MARGIN_X, y, SCALE);
        if (!row.value.empty())
        {
            const float vw = textWidth(row.value, SCALE);
            drawText(renderer, font, row.value,
                     static_cast<float>(LOGICAL_W) - STAT_MARGIN_X - vw, y, SCALE);
        }
        y += STAT_LINE_H;
    }
    drawText(renderer, font, "MONEY", STAT_MARGIN_X, y, SCALE);
    {
        const std::string m = std::to_string(money);
        drawText(renderer, font, m,
                 static_cast<float>(LOGICAL_W) - STAT_MARGIN_X - textWidth(m, SCALE), y, SCALE);
    }

    // --- Upgrade bar labels, aligned to each bar's on-screen rect: name at the left
    // edge, LV/cost status at the right edge. Uses the same transformToFrect as
    // drawSystem, so text tracks the fill. (NEXT DAY has no label — it's the
    // backdrop's power button, see the hit-box comment in onInit.) ---
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(shopMask)) continue;
        const ShopButton& b   = e.get<ShopButton>();
        const int         lvl = UpgradeState::level(b.id);
        const SDL_FRect   r   = transformToFrect(e.get<Transform>(), cam);
        const float       ty  = r.y + r.h * 0.5f + LABEL_Y_OFFSET;

        drawText(renderer, font, upgradeDef(b.id).name, r.x + LABEL_PAD, ty, SCALE);

        const std::string status = shopStatus(b.id, lvl);
        drawText(renderer, font, status,
                 r.x + r.w - LABEL_PAD - textWidth(status, SCALE), ty, SCALE);
    }

    SDL_RenderPresent(renderer);
    return true;
}

void DayReportScene::onCleanup()
{
    destroyAllGameEntities(); // clear the shop entities before the next scene
}
} // namespace cafe
