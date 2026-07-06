#include "HowToPlayScene.h"

#include "AssetManager.h"
#include "ButtonSystem.h"  // buttonDispatchSystem
#include "Components.h"    // Button, Transform, Drawable, TextLabel
#include "Entities.h"      // destroyAllGameEntities
#include "Glyph.h"         // GLYPH_H
#include "MenuSystem.h"    // menuInputSystem
#include "RenderContext.h"
#include "RenderLayers.h"
#include "RenderSystem.h"  // drawSystem, drawTextSystem
#include "Transform.h"     // screenToWorldPoint, worldToScreenPoint
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
namespace
{
constexpr auto FONT_TEX = "font.png";
constexpr int  SCALE    = 1;

// Layout (world units; camera at origin: 1 unit = 8 px, screen centre 80,45 =
// world 0,0, world y up). Screen-px equivalents in comments — same split
// StartMenuScene/DayReportScene use.

// BACK button: small, bottom-left corner.
constexpr WorldPos BACK_POS    = { -8.f, -4.25f };   // screen (16, 79), bottom-left
constexpr float    BTN_HALF_W  = 1.25f;              // 20 px wide
constexpr float    BTN_HALF_H  = 0.5f;               // 8 px tall

constexpr float LABEL_Y_OFFSET = -static_cast<float>(GLYPH_H) * 0.5f;
constexpr SDL_Color TEXT_ON_SQUARE = { 0, 0, 0, 255 }; // black text on white square

// How-to text: white lines centred on the screen's vertical axis (screen x=80),
// drawn straight on the black background. All-caps, only ' 0-9 A-Z / - %' glyphs
// exist (see Glyph.h), so no other punctuation. Title on top, body stacked below.
// Layout is screen-px (converted with screenToWorldPoint) — tweak the four
// constants below to taste; the body auto-stacks from BODY_TOP_Y by BODY_STEP.
constexpr SDL_Color TEXT_WHITE = { 255, 255, 255, 255 };
constexpr float     TITLE_SCALE = 1.4f;
constexpr float     TITLE_Y     = 5.f;   // screen y (top) of the title
constexpr float     BODY_SCALE  = 0.52f;
constexpr float     BODY_TOP_Y  = 16.f;  // screen y of the first body line
constexpr float     BODY_STEP   = 4.3f;  // screen-px between body lines (line gap)

constexpr const char* HOWTO_TITLE = "HOW TO PLAY";

// Body copy, top to bottom. Each string is one centred line (<=25 chars so it
// fits the 160px canvas at BODY_SCALE). 13 lines dense-stacked to fit the 90px
// canvas above the BACK button — grouping shows through the wording. (Add blank
// "" spacers back for section gaps if you shrink BODY_SCALE/STEP to make room.)
constexpr const char* HOWTO_LINES[] = {
    "SERVE EACH CUSTOMER ORDER",
    "BEFORE PATIENCE RUNS OUT",
    "CUP - HOLD COFFEE WATER",
    "MILK TO POUR THE MIX",
    "ICE MAKES A DRINK COLD",
    "MICROWAVE HEATS PASTRIES",
    "DRAG FINISHED ITEMS ONTO",
    "THE CUSTOMER TO SERVE",
    "FIRE ICON - SERVE HOT",
    "ICE ICON - SERVE COLD",
    "TEMP ICON RIGHT OF ITEM",
    "MATCH RECIPE AND TEMP",
    "NAPKIN SHOWS ALL RECIPES",
    "EARN MONEY - BUY UPGRADES",
    "BETWEEN DAYS",
};

/** White placeholder square (null texture => drawSystem fills the rect with tint).
 *  Swap to real art later by filling texture + srcRect. */
bagel::Entity makeSquare(WorldPos pos, float halfW, float halfH)
{
    auto e = bagel::Entity::create();
    e.addAll(
        Transform{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH },
        Drawable{ nullptr, SDL_FRect{}, layer::UI1 }); // default tint: opaque white
    return e;
}

/** Centred label over a square (text-over-bar layout from StartMenuScene). */
void addCenteredLabel(WorldPos squarePos, const char* text, WorldPos cam)
{
    const SDL_FPoint screen = worldToScreenPoint({ squarePos.x, squarePos.y }, cam);
    const WorldPos   pos    = screenToWorldPoint({ screen.x, screen.y + LABEL_Y_OFFSET }, cam);
    auto e = bagel::Entity::create();
    e.addAll(
        Transform{ .x = pos.x, .y = pos.y },
        TextLabel{ text, SCALE, TextAlign::Center, TEXT_ON_SQUARE });
}

/** One centred text line at screen (80, screenY), converted to world. Center
 *  alignment anchors it on the screen's vertical axis. */
void addCenteredLine(float screenY, const char* text, float scale, SDL_Color tint, WorldPos cam)
{
    const WorldPos pos = screenToWorldPoint({ 80.f, screenY }, cam); // 80 = screen centre x
    auto e = bagel::Entity::create();
    e.addAll(
        Transform{ .x = pos.x, .y = pos.y },
        TextLabel{ text, scale, TextAlign::Center, tint });
}
} // namespace

void HowToPlayScene::onInit()
{
    getAssetManager().getTexture(FONT_TEX); // warm the cache

    const WorldPos cam = RenderContext::getCameraPos();

    // Title + body: white centred lines on the black background. Blank entries in
    // HOWTO_LINES still consume a step, acting as section spacers.
    addCenteredLine(TITLE_Y, HOWTO_TITLE, TITLE_SCALE, TEXT_WHITE, cam);
    float y = BODY_TOP_Y;
    for (const char* line : HOWTO_LINES)
    {
        if (line[0] != '\0')
            addCenteredLine(y, line, BODY_SCALE, TEXT_WHITE, cam);
        y += BODY_STEP;
    }

    // BACK button: square + tag + centred label.
    makeSquare(BACK_POS, BTN_HALF_W, BTN_HALF_H)
        .add(Button{ .kind = ButtonKind::Menu, .menuAction = MenuAction::Back });
    addCenteredLabel(BACK_POS, "BACK", cam);
}

bool HowToPlayScene::onUpdate(float /*dt*/)
{
    SDL_Renderer* renderer = getRenderer();

    bool exit = false;
    menuInputSystem(renderer, exit, getAudioContext()); // window close -> exit; updates buttons from mouse

    // Thin per-scene Menu read: only Back is raised in this scene.
    bool back = false;
    static const bagel::Mask menuMask = bagel::MaskBuilder().set<Button>().build();
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(menuMask)) continue;
        auto& b = e.get<Button>();
        if (b.kind != ButtonKind::Menu || !b.pressed) continue;
        b.pressed = false;
        if (b.menuAction == MenuAction::Back) back = true;
    }

    buttonDispatchSystem(getAssetManager(), nullptr, getAudioContext()); // no Spawn buttons here

    if (exit) { requestNext(SceneId::Quit);      return false; }
    if (back) { requestNext(SceneId::StartMenu); return false; }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black background
    SDL_RenderClear(renderer);

    drawSystem(renderer);                                             // placeholder squares
    drawTextSystem(renderer, getAssetManager().getTexture(FONT_TEX)); // BACK label

    SDL_RenderPresent(renderer);
    return true;
}

void HowToPlayScene::onCleanup()
{
    destroyAllGameEntities(); // clear this scene's entities before the next scene
}
} // namespace cafe
