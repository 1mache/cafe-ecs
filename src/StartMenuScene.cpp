#include "StartMenuScene.h"

#include "AssetManager.h"
#include "ButtonSystem.h"  // buttonDispatchSystem
#include "Components.h"    // Button, Transform, Drawable, TextLabel
#include "Entities.h"      // destroyAllGameEntities, createBg
#include "Glyph.h"         // GLYPH_H
#include "MenuSystem.h"
#include "RenderContext.h"
#include "RenderLayers.h"
#include "RenderSystem.h"  // drawSystem, drawTextSystem
#include "SoundAssets.h"
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
constexpr float SCALE      = 0.75f; // a bit smaller than the default full-glyph size

// --- Layout. Squares are in WORLD units (camera at origin: 1 unit = 8 px,
// screen centre 80,45 = world 0,0, world y up); text anchors are in screen px
// on the 160x90 logical canvas, converted with screenToWorldPoint — the same
// split DayReportScene uses. Screen-px equivalents in the comments. ---

// Logo: centred above the buttons. Height derived from the art's aspect at
// runtime (see onInit) so the sprite never distorts — tweak POS/HALF_W to taste.
constexpr WorldPos LOGO_POS    = { 0.f, 2.75f };   // screen (80, 23)
constexpr float    LOGO_HALF_W = 3.0f;             // 48 px wide

constexpr WorldPos START_POS   = { 0.f, -0.875f }; // screen (80, 52)
constexpr WorldPos EXIT_POS    = { 0.f, -3.375f };  // screen (80, 72) — 20px below START, 4px clear of its 16px-tall art

// HOW-TO-PLAY (info) button: bottom-left corner.
constexpr WorldPos HOWTO_POS  = { -8.5f, -4.375f }; // screen (12, 80), bottom-left
constexpr float    HOWTO_HALF = 0.625f;            // 10 x 10 px

// Sound toggle: next to info, same y, one step right.
constexpr WorldPos SOUND_POS   = { -6.75f, -4.375f }; // screen (26, 80), right of info
constexpr float    SOUND_HALF  = 0.625f;              // 10 x 10 px

// Vertical centring for a label over a square (same trick as DayReportScene).
// Scaled by SCALE too, else it centres for scale-1 glyphs regardless of the
// label's actual (smaller) size.
constexpr float LABEL_Y_OFFSET = -static_cast<float>(GLYPH_H) * SCALE * 0.5f;

// White text over the ui_buttons2 art (was black, back when the buttons were plain white squares).
constexpr SDL_Color TEXT_ON_SQUARE = { 255, 255, 255, 255 };

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

    // START / EXIT buttons: ui_buttons2's long-button background (frame 0 idle,
    // frame 1 pressed — buttonDispatchSystem swaps it live), sized at native
    // resolution (texToWorldScale of the sheet's own frame size, not a tuned
    // constant), plus a centred label.
    {
        constexpr auto TEX_PATH   = "ui_buttons2.png";
        constexpr auto SHEET_PATH = "ui_buttons2.json";
        const SpriteSheet& sheet  = getAssetManager().getSpriteSheet(TEX_PATH, SHEET_PATH);
        const Texture&     tex    = getAssetManager().getTexture(TEX_PATH);
        const float halfW = texToWorldScale(sheet.spriteSize().x);
        const float halfH = texToWorldScale(sheet.spriteSize().y);

        auto makeMenuButton = [&](WorldPos pos, MenuAction action, const char* label)
        {
            auto e = bagel::Entity::create();
            e.addAll(
                Transform{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH },
                Drawable{ tex.get(), sheet.getFrameRect(0), layer::UI1 },
                Button{ .kind = ButtonKind::Menu, .menuAction = action });
            addCenteredLabel(pos, label, cam);
        };

        makeMenuButton(START_POS, MenuAction::Start, "START");
        makeMenuButton(EXIT_POS, MenuAction::Exit, "EXIT");
    }

    // Sound + info toggles: share ui_buttons.json's sheet. Neither has a label
    // (per the sketch); buttonDispatchSystem paints the soundon/soundoff frame
    // every frame for the sound button. Both kept at their existing on-screen
    // size (SOUND_HALF/HOWTO_HALF) rather than the sheet's native 16x16 frame.
    {
        constexpr auto TEX_PATH   = "ui_buttons.png";
        constexpr auto SHEET_PATH = "ui_buttons.json";
        const SpriteSheet& sheet  = getAssetManager().getSpriteSheet(TEX_PATH, SHEET_PATH);
        const Texture&     tex    = getAssetManager().getTexture(TEX_PATH);
        const auto [soundOnFrame, soundOnEnd] = sheet.getTagBounds("soundon");
        const auto [infoFrame, infoEnd]       = sheet.getTagBounds("info");

        auto sound = bagel::Entity::create();
        sound.addAll(
            Transform{ .x = SOUND_POS.x, .y = SOUND_POS.y, .w = SOUND_HALF, .h = SOUND_HALF },
            Drawable{ tex.get(), sheet.getFrameRect(soundOnFrame), layer::UI1 },
            Button{ .kind = ButtonKind::Sound });

        auto info = bagel::Entity::create();
        info.addAll(
            Transform{ .x = HOWTO_POS.x, .y = HOWTO_POS.y, .w = HOWTO_HALF, .h = HOWTO_HALF },
            Drawable{ tex.get(), sheet.getFrameRect(infoFrame), layer::UI1 },
            Button{ .kind = ButtonKind::Menu, .menuAction = MenuAction::HowToPlay });
    }

    getAudioContext().playMusic(sound::INTRO_MUSIC, sound::MUSIC_VOLUME);
}

bool StartMenuScene::onUpdate(float /*dt*/)
{
    SDL_Renderer* renderer = getRenderer();

    bool exit = false;
    menuInputSystem(renderer, exit, getAudioContext());

    // Thin per-scene Menu read: map Button{kind==Menu}.menuAction -> start/exit/how-to.
    bool start = false;
    bool howto = false;
    static const bagel::Mask menuMask = bagel::MaskBuilder().set<Button>().build();
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(menuMask)) continue;
        auto& b = e.get<Button>();
        if (b.kind != ButtonKind::Menu || !b.pressed) continue;
        b.pressed = false;
        switch (b.menuAction)
        {
        case MenuAction::Start:     start = true; break;
        case MenuAction::Exit:      exit  = true; break;
        case MenuAction::HowToPlay: howto = true; break;
        default: break; // NextDay / Back are not raised in the start menu
        }
    }

    buttonDispatchSystem(getAssetManager(), nullptr, getAudioContext(), renderer); // no PhysicsContext here: no Spawn buttons in this scene

    if (exit)  { requestNext(SceneId::Quit);      return false; }
    if (start) { requestNext(SceneId::MainGame);  return false; }
    if (howto) { requestNext(SceneId::HowToPlay); return false; }

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
