#include "StartMenuScene.h"

#include "Components.h"    // StartButton, ExitButton, SoundToggleButton, Transform, Drawable, TextLabel
#include "Entities.h"      // destroyAllGameEntities
#include "GameConfig.h"    // LOGICAL_W
#include "Glyph.h"         // GLYPH_H
#include "MenuSystem.h"
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

// --- Layout. Squares are in WORLD units (camera at origin: 1 unit = 8 px,
// screen centre 80,45 = world 0,0, world y up); text anchors are in screen px
// on the 160x90 logical canvas, converted with screenToWorldPoint — the same
// split DayReportScene uses. Screen-px equivalents in the comments. ---
constexpr float TITLE_Y = 8.f;   // screen px; title is centred on x = 80

constexpr WorldPos LOGO_POS    = { 0.f, 1.625f };  // screen (80, 32)
constexpr float    LOGO_HALF_W = 2.5f;             // 40 px wide
constexpr float    LOGO_HALF_H = 1.25f;            // 20 px tall

constexpr float    BTN_HALF_W  = 1.875f;           // 30 px wide
constexpr float    BTN_HALF_H  = 0.625f;           // 10 px tall
constexpr WorldPos START_POS   = { 0.f, -0.875f }; // screen (80, 52)
constexpr WorldPos EXIT_POS    = { 0.f, -2.5f };   // screen (80, 65)

constexpr WorldPos SOUND_POS   = { 8.5f, -4.375f };// screen (148, 80), bottom-right
constexpr float    SOUND_HALF  = 0.625f;           // 10 x 10 px

// Vertical centring for a label over a square (same trick as DayReportScene).
constexpr float LABEL_Y_OFFSET = -static_cast<float>(GLYPH_H) * 0.5f;

// Font glyphs are white; black text keeps the labels readable on white squares.
constexpr SDL_Color TEXT_ON_SQUARE = { 0, 0, 0, 255 };
constexpr SDL_Color BG_TINT        = { 0, 0, 0, 255 };

/** White placeholder square: null texture => drawSystem fills the rect with
 *  tint. Swap to real art later by filling texture + srcRect (the microwave
 *  placeholder workflow). */
bagel::Entity makeSquare(WorldPos pos, float halfW, float halfH)
{
    auto e = bagel::Entity::create();
    e.addAll(
        Transform{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH },
        Drawable{ nullptr, SDL_FRect{}, layer::UI1 } // default tint: opaque white
    );
    return e;
}

/** Full-canvas black placeholder background — same null-texture-quad idiom as
 *  every other placeholder here, sized like createBg (CafeEnvironmentFactory)
 *  so swapping in real art later is either filling this Drawable's texture +
 *  srcRect, or replacing this call with createBg(assets, "menu_bg.png"). Drawn
 *  at layer::BG so drawSystem paints it first and it covers the whole canvas
 *  every frame — no separate clear-colour bookkeeping needed. */
bagel::Entity makeBgPlaceholder()
{
    auto e = bagel::Entity::create();
    e.addAll(
        Transform{ .x = 0.f, .y = 0.f,
                   .w = texToWorldScale(static_cast<float>(LOGICAL_W)),
                   .h = texToWorldScale(static_cast<float>(LOGICAL_H)) },
        Drawable{ nullptr, SDL_FRect{}, layer::BG, BG_TINT }
    );
    return e;
}

/** Centred label over a square: its own TextLabel entity at the square's
 *  centre, nudged up half a glyph (text-over-bar layout from DayReportScene). */
void addCenteredLabel(WorldPos squarePos, const char* text, WorldPos cam)
{
    const SDL_FPoint screen = worldToScreenPoint({ squarePos.x, squarePos.y }, cam);
    const WorldPos   pos    = screenToWorldPoint({ screen.x, screen.y + LABEL_Y_OFFSET }, cam);
    auto e = bagel::Entity::create();
    e.addAll(
        Transform{ .x = pos.x, .y = pos.y },
        TextLabel{ text, SCALE, TextAlign::Center, TEXT_ON_SQUARE });
}
} // namespace

void StartMenuScene::onInit()
{
    getAssetManager().getTexture(FONT_TEX); // warm the cache

    const WorldPos cam = RenderContext::getCameraPos();

    // Background placeholder: plain black, covers the whole canvas.
    makeBgPlaceholder();

    // Title, top-centre.
    {
        const WorldPos pos = screenToWorldPoint(
            { static_cast<float>(LOGICAL_W) * 0.5f, TITLE_Y }, cam);
        auto e = bagel::Entity::create();
        e.addAll(
            Transform{ .x = pos.x, .y = pos.y },
            TextLabel{ "ENTITY COFFEE SYSTEM", SCALE, TextAlign::Center });
    }

    // Logo placeholder: plain white square, no behaviour.
    makeSquare(LOGO_POS, LOGO_HALF_W, LOGO_HALF_H);

    // START / EXIT buttons: square + tag + centred label.
    makeSquare(START_POS, BTN_HALF_W, BTN_HALF_H).add(StartButton{});
    addCenteredLabel(START_POS, "START", cam);

    makeSquare(EXIT_POS, BTN_HALF_W, BTN_HALF_H).add(ExitButton{});
    addCenteredLabel(EXIT_POS, "EXIT", cam);

    // Sound toggle: no label (per the sketch); soundToggleSystem paints its
    // on/off tint every frame, including the first.
    makeSquare(SOUND_POS, SOUND_HALF, SOUND_HALF).add(SoundToggleButton{});
}

bool StartMenuScene::onUpdate(float /*dt*/)
{
    SDL_Renderer* renderer = getRenderer();

    bool start = false, exit = false;
    menuInputSystem(renderer, start, exit);
    soundToggleSystem(getAudioContext());

    if (exit)  { requestNext(SceneId::Quit);     return false; }
    if (start) { requestNext(SceneId::MainGame); return false; }

    SDL_RenderClear(renderer);

    drawSystem(renderer);                                          // bg + squares
    drawTextSystem(renderer, getAssetManager().getTexture(FONT_TEX)); // title + labels

    SDL_RenderPresent(renderer);
    return true;
}

void StartMenuScene::onCleanup()
{
    destroyAllGameEntities(); // clear the menu entities before the next scene
}
} // namespace cafe
