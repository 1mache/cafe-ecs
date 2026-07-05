#include "ButtonSystem.h"

#include "AudioContext.h"
#include "Components.h"
#include "Entities.h"       // createCup, createIceCube
#include "SettingsState.h"
#include "SoundAssets.h"
#include "Transform.h"      // isPointInsideTransform
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

void buttonDispatchSystem(AssetManager& assets, PhysicsContext* physics, AudioContext& audio)
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
                audio.play(sound::BUY_UPGRADE);
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
}
} // namespace cafe
