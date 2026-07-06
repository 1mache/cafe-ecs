#include "ButtonSystem.h"

#include "AssetManager.h"
#include "AudioContext.h"
#include "Components.h"
#include "Entities.h"       // createCup, createIceCube
#include "IntentSystem.h"   // mouseWindowToRenderPoint
#include "RenderContext.h"
#include "SettingsState.h"
#include "SoundAssets.h"
#include "SpriteSheet.h"
#include "Transform.h"      // isPointInsideTransform, screenToWorldPoint
#include "UpgradeState.h"

#include <bagel.h>
#include <cassert>
#include <utility>
#include <vector>

namespace cafe
{
void updateButtonsFromMouse(WorldPos worldMouse, bool clicked, bool mouseUp, AudioContext& audio)
{
    if (!clicked && !mouseUp) return;

    static const bagel::Mask buttonMask =
        bagel::MaskBuilder().set<Button>().set<Transform>().build();

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(buttonMask)) continue;

        auto& b = e.get<Button>();

        if (b.kind == ButtonKind::Machine)
        {
            // Held: press on down-inside, release on any mouse-up.
            if (clicked && isPointInsideTransform(worldMouse, e.get<Transform>()))
            {
                b.pressed = true;
                audio.play(sound::BUTTON_PRESS, sound::BUTTON_VOLUME);
            }
            else if (mouseUp)
            {
                b.pressed = false;
            }
        }
        else if (clicked && isPointInsideTransform(worldMouse, e.get<Transform>()))
        {
            // Momentary: press on down-inside; consumer clears it.
            b.pressed = true;
            audio.play(sound::BUTTON_PRESS, sound::BUTTON_VOLUME);
        }
    }
}

void buttonDispatchSystem(AssetManager& assets, PhysicsContext* physics, AudioContext& audio,
                          SDL_Renderer* renderer)
{
    static const bagel::Mask buttonMask = bagel::MaskBuilder().set<Button>().build();

    std::vector<std::pair<WorldPos, DropType>> toSpawn;

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(buttonMask)) continue;

        auto& b = e.get<Button>();
        if (!b.pressed) continue;

        switch (b.kind)
        {
        case ButtonKind::Shop:
            b.pressed = false;
            if (UpgradeState::tryBuy(b.upgradeId)) // no-op + false if unaffordable or maxed
                audio.play(sound::BUY_UPGRADE, sound::UPGRADE_VOLUME);
            break;

        case ButtonKind::Sound:
            b.pressed = false;
            SettingsState::toggleMuted();
            audio.setVolume(SettingsState::muted() ? 0.f : 1.f);
            break;

        case ButtonKind::Spawn:
            // Pastry has its own pastrySupplySystem; don't consume its click here.
            if (b.dropType == DropType::Pastry) continue;
            if (!consumeSpawnRequest(b)) continue; // clears pressed either way
            assert(physics && "buttonDispatchSystem: Spawn button pressed with no PhysicsContext");
            toSpawn.emplace_back(b.spawnPos, b.dropType);
            break;

        case ButtonKind::Menu:
        case ButtonKind::Machine:
        default:
            break; // handled elsewhere (per-scene Menu read / machineButtonSystem)
        }
    }

    for (auto& sp : toSpawn)
    {
        if (sp.second == DropType::Cup)
            createCup(assets, *physics, sp.first);
        else if (sp.second == DropType::Ice)
            createIceCube(assets, *physics, sp.first);
    }

    // Live sprite-state refresh: independent of `pressed` above (already consumed
    // for Spawn by the time we get here), so cup/sound art reflects the current
    // frame's real state rather than a one-frame-old edge.
    static const bagel::Mask spriteMask =
        bagel::MaskBuilder().set<Button>().set<Drawable>().set<Transform>().build();

    constexpr auto UI_TEX_PATH   = "ui_buttons.png";
    constexpr auto UI_SHEET_PATH = "ui_buttons.json";
    const SpriteSheet& uiSheet = assets.getSpriteSheet(UI_TEX_PATH, UI_SHEET_PATH);

    // Start/Exit long-button background: no frame tags, just idle (0) / pressed (1).
    constexpr auto UI2_TEX_PATH   = "ui_buttons2.png";
    constexpr auto UI2_SHEET_PATH = "ui_buttons2.json";
    const SpriteSheet& menuSheet = assets.getSpriteSheet(UI2_TEX_PATH, UI2_SHEET_PATH);

    float mx{}, my{};
    const bool mouseHeld = (SDL_GetMouseState(&mx, &my) & SDL_BUTTON_LMASK) != 0;
    const WorldPos worldMouse =
        screenToWorldPoint(mouseWindowToRenderPoint(renderer, mx, my), RenderContext::getCameraPos());

    const auto [cupIdle, cupPressed]  = uiSheet.getTagBounds("cup");
    const auto [soundOnFrame, soundOnEnd]   = uiSheet.getTagBounds("soundon");
    const auto [soundOffFrame, soundOffEnd] = uiSheet.getTagBounds("soundoff");
    (void)soundOnEnd; (void)soundOffEnd; // single-frame tags; only .first is used

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(spriteMask)) continue;
        auto& b = e.get<Button>();
        auto& d = e.get<Drawable>();

        if (b.kind == ButtonKind::Spawn && b.dropType == DropType::Cup)
        {
            const bool showPressed = mouseHeld && isPointInsideTransform(worldMouse, e.get<Transform>());
            d.srcRect = uiSheet.getFrameRect(showPressed ? cupPressed : cupIdle);
        }
        else if (b.kind == ButtonKind::Sound)
        {
            d.srcRect = uiSheet.getFrameRect(SettingsState::muted() ? soundOffFrame : soundOnFrame);
        }
        else if (b.kind == ButtonKind::Menu)
        {
            const bool showPressed = mouseHeld && isPointInsideTransform(worldMouse, e.get<Transform>());
            d.srcRect = menuSheet.getFrameRect(showPressed ? 1 : 0);
        }
    }
}
} // namespace cafe
