#include "DayReportScene.h"

#include "ButtonSystem.h"      // buttonDispatchSystem
#include "Components.h"        // Button, Transform, Drawable
#include "DayReport.h"
#include "DayState.h"
#include "Entities.h"          // destroyAllGameEntities
#include "GameConfig.h"
#include "Glyph.h"             // GLYPH_H
#include "RenderContext.h"
#include "RenderLayers.h"
#include "RenderSystem.h"      // drawSystem, drawTextSystem
#include "ShopSystem.h"
#include "SoundAssets.h"
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
// Laid out as a 2x2 grid inside the backdrop's black screen area (see above),
// clear of its bottom bezel (screen y >= 76). ---
constexpr float BTN_HALF_W  = 4.375f;  // upgrade bars: ~70 px wide (two columns fit)
constexpr float BTN_HALF_H  = 0.5625f; // ~9 px tall (one text line + padding)
constexpr float BTN_X       = 0.f;     // grid centered horizontally
constexpr float BTN_COL_DX  = 4.75f;   // +/- column offset from center (~38 px)
constexpr float FIRST_BTN_Y = -1.5625f; // top row (~screen y 57.5)
constexpr float BTN_ROW_GAP = 1.5f;     // world gap between the two rows (~12 px)

// The backdrop already draws a red "power" button in its bottom-right bezel
// (screen px center ~149,84, ~9 px across); this is just its click hit-box —
// no Drawable, so nothing is rendered over the baked-in art.
constexpr WorldPos NEXT_DAY_HITBOX_POS  = { 8.625f, -4.875f };
constexpr float    NEXT_DAY_HITBOX_HALF = 0.625f; // ~5 px half-size, a bit larger than the icon

constexpr float BTN_FONT_SCALE = 0.45f;                               // shrunk font for the grid's name/status labels
constexpr float LABEL_PAD      = 3.f;                                 // text inset from bar edge
constexpr float LABEL_Y_OFFSET = -static_cast<float>(GLYPH_H) * BTN_FONT_SCALE * 0.5f; // vertical centring

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

    const WorldPos cam = RenderContext::getCameraPos();

    // --- Stats block: DAY/SERVED/LOST/SCORE, then MONEY. Rows are static for
    // the lifetime of this scene (DayState is frozen once the day is over), so
    // these are created once here and never touched again in onUpdate. ---
    float y = STAT_TOP_Y;
    for (const auto& row : buildReportRows())
    {
        if (row.label.empty() || row.label == "CLICK TO CONTINUE")
            continue;

        const WorldPos labelPos = screenToWorldPoint({ STAT_MARGIN_X, y }, cam);
        auto labelEnt = bagel::Entity::create();
        labelEnt.addAll(
            Transform{ .x = labelPos.x, .y = labelPos.y },
            TextLabel{ row.label, SCALE, TextAlign::Left });

        if (!row.value.empty())
        {
            const WorldPos valuePos = screenToWorldPoint(
                { static_cast<float>(LOGICAL_W) - STAT_MARGIN_X, y }, cam);
            auto valueEnt = bagel::Entity::create();
            valueEnt.addAll(
                Transform{ .x = valuePos.x, .y = valuePos.y },
                TextLabel{ row.value, SCALE, TextAlign::Right });
        }
        y += STAT_LINE_H;
    }

    // MONEY row: label is static, value changes when the player buys an upgrade
    // (mutated in onUpdate via the MoneyLabel tag).
    {
        const WorldPos labelPos = screenToWorldPoint({ STAT_MARGIN_X, y }, cam);
        auto labelEnt = bagel::Entity::create();
        labelEnt.addAll(
            Transform{ .x = labelPos.x, .y = labelPos.y },
            TextLabel{ "MONEY", SCALE, TextAlign::Left });

        const WorldPos valuePos = screenToWorldPoint(
            { static_cast<float>(LOGICAL_W) - STAT_MARGIN_X, y }, cam);
        auto valueEnt = bagel::Entity::create();
        valueEnt.addAll(
            Transform{ .x = valuePos.x, .y = valuePos.y },
            TextLabel{ std::to_string(UpgradeState::money()), SCALE, TextAlign::Right },
            MoneyLabel{});
    }

    // One clickable bar per upgrade (world-space entities, drawn as filled bars),
    // plus its two labels: name (static) and LV/cost status (mutated in
    // onUpdate via the UpgradeStatusLabel tag).
    for (int i = 0; i < static_cast<int>(UpgradeId::count); ++i)
    {
        const int      col = i % 2;             // 0 = left, 1 = right
        const int      row = i / 2;             // 0 = top,  1 = bottom
        const WorldPos pos{ BTN_X + (col == 0 ? -BTN_COL_DX : BTN_COL_DX),
                            FIRST_BTN_Y - static_cast<float>(row) * BTN_ROW_GAP };
        const auto     id = static_cast<UpgradeId>(i);

        auto e = makeButtonEntity(pos, BTN_HALF_W, BTN_HALF_H);
        e.add(Button{ .kind = ButtonKind::Shop, .upgradeId = id });

        const SDL_FRect r  = transformToFrect(
            Transform{ .x = pos.x, .y = pos.y, .w = BTN_HALF_W, .h = BTN_HALF_H }, cam);
        const float     ty = r.y + r.h * 0.5f + LABEL_Y_OFFSET;

        const WorldPos namePos = screenToWorldPoint({ r.x + LABEL_PAD, ty }, cam);
        auto nameEnt = bagel::Entity::create();
        nameEnt.addAll(
            Transform{ .x = namePos.x, .y = namePos.y },
            TextLabel{ upgradeDef(id).name, BTN_FONT_SCALE, TextAlign::Left });

        const WorldPos statusPos = screenToWorldPoint({ r.x + r.w - LABEL_PAD, ty }, cam);
        auto statusEnt = bagel::Entity::create();
        statusEnt.addAll(
            Transform{ .x = statusPos.x, .y = statusPos.y },
            TextLabel{ shopStatus(id, UpgradeState::level(id)), BTN_FONT_SCALE, TextAlign::Right },
            UpgradeStatusLabel{ id });
    }

    // Next-day trigger: a Transform-only hit-box over the backdrop's baked-in power
    // button. No Drawable, so updateButtonsFromMouse still hit-tests it (Transform-only
    // mask) but drawSystem has nothing to draw for it.
    auto next = bagel::Entity::create();
    next.addAll(
        Transform{ .x = NEXT_DAY_HITBOX_POS.x, .y = NEXT_DAY_HITBOX_POS.y,
                   .w = NEXT_DAY_HITBOX_HALF, .h = NEXT_DAY_HITBOX_HALF },
        Button{ .kind = ButtonKind::Menu, .menuAction = MenuAction::NextDay }
    );

    getAudioContext().playMusic(sound::MAIN_MUSIC_2, sound::MUSIC_VOLUME);
}

bool DayReportScene::onUpdate(float /*dt*/)
{
    SDL_Renderer* renderer = getRenderer();
    auto&         assets   = getAssetManager();

    bool nextDay = false, exitRequested = false;
    shopInputSystem(renderer, nextDay, exitRequested, getAudioContext());

    // Thin per-scene Menu read: only NextDay is raised in the day-report scene.
    static const bagel::Mask menuMask = bagel::MaskBuilder().set<Button>().build();
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(menuMask)) continue;
        auto& b = e.get<Button>();
        if (b.kind != ButtonKind::Menu || !b.pressed) continue;
        b.pressed = false;
        if (b.menuAction == MenuAction::NextDay) nextDay = true;
    }

    buttonDispatchSystem(getAssetManager(), nullptr, getAudioContext()); // no PhysicsContext here: no Spawn buttons in this scene

    if (exitRequested) { requestNext(SceneId::Quit);     return false; }
    if (nextDay)       { requestNext(SceneId::MainGame); return false; }

    static const bagel::Mask shopMask =
        bagel::MaskBuilder().set<Button>().set<Transform>().set<Drawable>().build();
    static const bagel::Mask moneyLabelMask =
        bagel::MaskBuilder().set<MoneyLabel>().set<TextLabel>().build();
    static const bagel::Mask statusLabelMask =
        bagel::MaskBuilder().set<UpgradeStatusLabel>().set<TextLabel>().build();

    const int money = UpgradeState::money();

    // Colour each upgrade bar by its purchase state, then let drawSystem fill it.
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(shopMask)) continue;
        const Button& b = e.get<Button>();
        if (b.kind != ButtonKind::Shop) continue;
        const int lvl = UpgradeState::level(b.upgradeId);
        e.get<Drawable>().tint = isMaxed(b.upgradeId, lvl) ? COLOR_MAXED
                               : (money >= upgradeCost(b.upgradeId, lvl) ? COLOR_AFFORD : COLOR_POOR);
    }

    // Refresh the money value label.
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(moneyLabelMask)) continue;
        e.get<TextLabel>().setText(std::to_string(money));
    }

    // Refresh each upgrade's LV/cost status label.
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(statusLabelMask)) continue;
        const UpgradeId id = e.get<UpgradeStatusLabel>().id;
        e.get<TextLabel>().setText(shopStatus(id, UpgradeState::level(id)));
    }

    SDL_RenderClear(renderer);

    drawSystem(renderer); // backdrop + upgrade bars (world-space, engine render path)
    drawTextSystem(renderer, assets.getTexture(FONT_TEX)); // all report + label text

    SDL_RenderPresent(renderer);
    return true;
}

void DayReportScene::onCleanup()
{
    destroyAllGameEntities(); // clear the shop entities before the next scene
}
} // namespace cafe
