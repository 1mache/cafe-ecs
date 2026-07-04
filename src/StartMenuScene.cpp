#include "StartMenuScene.h"

#include "AssetManager.h"
#include "Components.h"    // MenuButton, SoundToggleButton, Transform, Drawable, TextLabel
#include "Entities.h"      // destroyAllGameEntities, createBg
#include "GameConfig.h"    // LOGICAL_W
#include "Glyph.h"         // GLYPH_H
#include "MenuSystem.h"
#include "RenderContext.h"
#include "RenderLayers.h"
#include "RenderSystem.h"  // drawSystem, drawTextSystem
#include "Texture.h"       // Texture::get, getFullSrcRect
#include "Transform.h"     // screenToWorldPoint, worldToScreenPoint
#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
namespace
{
constexpr auto FONT_TEX    = "font.png";
constexpr auto MENU_BG_TEX = "bg_big.png"; // shared cafe backdrop, authored oversized (createBg scales it down)
constexpr auto LOGO_TEX    = "logo.png";
constexpr int  SCALE       = 1;

// --- Layout. Squares are in WORLD units (camera at origin: 1 unit = 8 px,
// screen centre 80,45 = world 0,0, world y up); text anchors are in screen px
// on the 160x90 logical canvas, converted with screenToWorldPoint — the same
// split DayReportScene uses. Screen-px equivalents in the comments. ---
constexpr float TITLE_Y = 8.f;   // screen px; title is centred on x = 80

// Logo: centred above the buttons. Height derived from the art's aspect at
// runtime (see onInit) so the sprite never distorts — tweak POS/HALF_W to taste.
constexpr WorldPos LOGO_POS    = { 0.f, 2.375f };  // screen (80, 26)
constexpr float    LOGO_HALF_W = 2.25f;            // 36 px wide

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

    // Background: dima's full-canvas cafe backdrop (createBg scales the oversized
    // art down into the logical screen, same idiom as the day-report/game bg).
    createBg(getAssetManager(), MENU_BG_TEX);

    // Title, top-centre.
    {
        const WorldPos pos = screenToWorldPoint(
            { static_cast<float>(LOGICAL_W) * 0.5f, TITLE_Y }, cam);
        auto e = bagel::Entity::create();
        e.addAll(
            Transform{ .x = pos.x, .y = pos.y },
            TextLabel{ "ENTITY COFFEE SYSTEM", SCALE, TextAlign::Center });
    }

    // Logo: dima's art. Full texture scaled into a box whose height matches the
    // sprite's aspect ratio, so it never stretches (tweak LOGO_POS/LOGO_HALF_W).
    {
        const Texture&  logoTex = getAssetManager().getTexture(LOGO_TEX);
        const SDL_FRect src     = logoTex.getFullSrcRect();
        const float     halfH   = src.w > 0.f ? LOGO_HALF_W * (src.h / src.w) : LOGO_HALF_W;
        auto e = bagel::Entity::create();
        e.addAll(
            Transform{ .x = LOGO_POS.x, .y = LOGO_POS.y, .w = LOGO_HALF_W, .h = halfH },
            Drawable{ logoTex.get(), src, layer::UI1 });
    }

    // START / EXIT buttons: square + tag + centred label.
    makeSquare(START_POS, BTN_HALF_W, BTN_HALF_H).add(MenuButton{ MenuAction::Start });
    addCenteredLabel(START_POS, "START", cam);

    makeSquare(EXIT_POS, BTN_HALF_W, BTN_HALF_H).add(MenuButton{ MenuAction::Exit });
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
