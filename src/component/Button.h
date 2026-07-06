#pragma once

#include "DropType.h"
#include "Ingredient.h"
#include "UpgradeCatalog.h"
#include "WorldPos.h"
#include <bagel.h>
#include <box2d/id.h>

namespace cafe
{
/** @brief What a Button{kind==Menu} click signals to its scene. */
enum class MenuAction
{
    Start,     // start-menu START square -> begin the game
    Exit,      // start-menu EXIT square  -> quit
    NextDay,   // day-report power button -> advance to the next day
    HowToPlay, // start-menu HOW-TO square -> open the how-to-play scene
    Back,      // how-to-play BACK square  -> return to the start menu
};

/** @brief Which button role an entity plays. Selects which fields below are live. */
enum class ButtonKind
{
    Menu,
    Shop,
    Sound,
    Spawn,
    Machine,
};

/** @brief Unified clickable-Transform component, replacing MenuButton / ShopButton /
 *  SoundToggleButton / SpawnButton / MachineButton. `kind` discriminates which of the
 *  payload fields below are meaningful for a given instance; the rest stay zero.
 *  `pressed` is set by updateButtonsFromMouse (momentary for all kinds except Machine,
 *  which is held: set on mouse-down, cleared on mouse-up) and consumed by whichever
 *  system owns that kind (buttonDispatchSystem for Shop/Sound/Spawn, machineButtonSystem
 *  for Machine, the owning scene's Menu read for Menu). */
struct Button
{
    ButtonKind kind{};
    bool       pressed{};

    MenuAction       menuAction{};      // Menu
    UpgradeId        upgradeId{};       // Shop
    DropType         dropType{};        // Spawn
    LiquidIngredient liquid{};          // Machine
    b2ShapeId        spawnSlotSensor{}; // Spawn
    WorldPos         spawnPos{};        // Spawn
};

/** @brief True if the Spawn button was clicked this frame AND its spawn slot is free
 *  (nothing occupying it, so we don't spawn objects inside each other). Always
 *  consumes the click (clears pressed), whether or not it spawns. */
bool consumeSpawnRequest(Button& button);
} // namespace cafe

template <> struct bagel::Storage<cafe::Button> final : NoInstance { using type = SparseStorage<cafe::Button>; };
