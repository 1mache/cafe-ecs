# Button Unification — Refactor Results

Done. Five button component structs collapsed into one `Button`; the four
copy-pasted mouse hit-tests collapsed into one helper; the scattered
buy/spawn/mute-toggle logic folded into one dispatch system. Build verified
clean (`cmake --build build-x64-debug --target main`, 0 errors).

## Final shape

```cpp
enum class ButtonKind { Menu, Shop, Sound, Spawn, Machine };

struct Button
{
    ButtonKind       kind{};
    bool             pressed{};

    MenuAction       menuAction{};      // Menu
    UpgradeId        upgradeId{};       // Shop
    DropType         dropType{};        // Spawn
    LiquidIngredient liquid{};          // Machine
    b2ShapeId        spawnSlotSensor{}; // Spawn
    WorldPos         spawnPos{};        // Spawn
};
```

- `src/component/Button.h` / `.cpp` — the struct above, `SparseStorage`,
  `MenuAction` (moved in from the old `MenuButton.h`), and
  `consumeSpawnRequest(Button&)` (moved from `SpawnButton.cpp`, now
  guarding/clearing `pressed` instead of `justPressed`).
- Both old flag conventions (`justPressed` edge-triggered vs. `pressed`
  latched) collapsed into a single `pressed` field; `updateButtonsFromMouse`
  decides momentary-vs-held per `kind`.

## New system — `src/system/ButtonSystem.h` / `.cpp`

**`updateButtonsFromMouse(WorldPos worldMouse, bool clicked, bool mouseUp, AudioContext&)`**
— the one shared hit-test, replacing the four copies that used to live in
`updateMachineButtonIntent`, `updateSpawnButtonIntent` (`IntentSystem.cpp`),
`menuInputSystem`, and `shopInputSystem`. Each scene still polls SDL itself
(one poller per scene, unchanged); they compute the world-space mouse point
and call this helper once. Machine is held (press on down-inside, release on
any mouse-up); every other kind is momentary (press on down-inside, cleared
by its consumer).

**`buttonDispatchSystem(AssetManager&, PhysicsContext*, AudioContext&)`** —
one-shot consume of every pressed non-Menu, non-Machine button:
- Shop: `UpgradeState::tryBuy` + buy sound (folds the deleted `shopPurchaseSystem`).
- Sound: `SettingsState::toggleMuted` + volume (folds the toggle action out of
  `soundToggleSystem`, which is now tint-only).
- Spawn (skipping `dropType==Pastry`, owned by `pastrySupplySystem`):
  `consumeSpawnRequest` then `createCup`/`createIceCube` (folds the deleted
  `supplyButtonSystem`).

**Deviation from the original design doc:** `physics` is `PhysicsContext*`,
not `&`, and nullable. The original plan assumed every call site could supply
a `PhysicsContext&`, but `PhysicsContext` is only ever owned by
`MainGameScene` (`_physics` member) — `StartMenuScene` and `DayReportScene`
have none, and each still had to call `buttonDispatchSystem` for their own
Sound/Shop buttons. Only the Spawn case touches `physics`, so it's an
`assert`ed-non-null pointer: `MainGameScene` passes `&_physics`;
`StartMenuScene`/`DayReportScene` (no Spawn buttons ever) pass `nullptr`.

## Deleted

- Components: `MenuButton.h`, `ShopButton.h`, `SoundToggleButton.h`,
  `SpawnButton.h/.cpp`, `MachineButton.h`.
- Systems: `shopPurchaseSystem`, `supplyButtonSystem` (and the now-empty
  `SupplySystem.cpp` that held only the latter).

## Moved

`SupplySystem.h` held nothing but the `supply::` layout constants once
`supplyButtonSystem` was folded away — no longer a system, so it moved out of
`src/system/` to `src/Supply.h` (root, alongside the other constants-only
headers like `GameConfig.h`). Every includer (`MainGameScene.cpp`,
`SpawnButtonFactory.cpp`, `MicrowaveSystem.cpp`) updated; `Systems.h` no
longer pulls it in since it isn't a system.

## Touched, not deleted

- `IntentSystem.cpp`: `updateMachineButtonIntent`/`updateSpawnButtonIntent`
  removed; `intentSystem` now calls `updateButtonsFromMouse` once after
  building `UserInput`.
- `MenuSystem.cpp/.h`: `menuInputSystem` thinned to poll + `updateButtonsFromMouse`
  (Menu→Start/Exit mapping moved to a thin per-scene read in
  `StartMenuScene::onUpdate`). `soundToggleSystem` lost its parameter and its
  toggle action — tint only now, reading `SettingsState::muted()`.
- `ShopSystem.cpp/.h`: `shopInputSystem` thinned the same way (NextDay mapping
  moved to `DayReportScene::onUpdate`); `shopPurchaseSystem` deleted.
- `CoffeeMachineSystem.cpp`: `machineButtonSystem` masks `Button` filtered to
  `kind==Machine`, reads `button.liquid` (was `button.kind`).
- `PastrySupplySystem.cpp`: masks `PastryTv+Button` filtered to
  `kind==Spawn`, unchanged otherwise.
- Factories: `SpawnButtonFactory.cpp` and `CoffeeMachineFactory.cpp:61` build
  `Button{...}` instead of the old structs; `PastryTvFactory`/`IceMachineFactory`
  untouched (they call `createSpawnButton`, which absorbed the change).
- `Components.h`, `Systems.h`, `CMakeLists.txt`: updated include/source lists.

## Behavior changes

- Menu buttons (Start/Exit/NextDay) now play `BUTTON_PRESS` on click — the old
  `menuInputSystem` was silent. Every button kind now clicks the same way.
- `MachineButton`'s `StackStorage` → `Button`'s `SparseStorage`: no functional
  effect, buttons are few.

## Verification

- `cmake --build build-x64-debug --target main` — clean, 0 errors/warnings
  from the touched files.
- Component count: 41 headers in `src/component/` post-refactor (was 45
  pre-refactor, net −4 as designed), comfortably under bagel's 64-component
  cap.
- Not yet manually played — exercise each button (Start/Exit/sound toggle,
  cup/ice/pastry spawn, coffee-machine hold-to-pour, shop buy, NextDay) before
  merging.
