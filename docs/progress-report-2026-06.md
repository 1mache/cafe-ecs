# Progress Report — June 2026

## Overview

### B. Button-summoned cup/pastry supply — omerzv

| Hash | Date | Subject |
|------|------|---------|
| `e7e2f4f` | 2026-06-15 | feat: button-summoned cup/pastry supply with drop-in + destroy on leave |
| `f4658cc` | 2026-06-15 | merge: integrate origin/dima into supply feature |
| `56c114b` | 2026-06-15 | chore: remove unused includes after the dima merge |
| `6ba91e0` | 2026-06-16 | Merge commit into EitanK |

**`e7e2f4f` — Supply on demand (~369 insertions)**

Full supply system: clicking counter buttons spawns cups/pastries into the first free slot. Items fall in with an easeOutBack tween (`Falling` component) and become draggable on landing. Delivered items are tracked with `DeliveredTo` and destroyed when the customer leaves (`clearDeliveredItems`) — each cup's liquid drops cleaned up via `Liquid.owner` scoping. Removed the old fixed-item recycling approach.

Files changed:
- `src/CMakeLists.txt`
- `src/MainGameScene.cpp` / `.h`
- `src/component/Components.h`
- `src/component/DeliveredTo.h` *(new)*
- `src/component/Falling.h` *(new)*
- `src/component/Liquid.h` — added `owner` field
- `src/component/SpawnButton.h` *(new)*
- `src/entity/ButtonFactory.cpp` *(new)*
- `src/entity/SpawnButtonFactory.h` *(new)*
- `src/entity/Entities.h`
- `src/system/CustomerSystem.cpp` / `.h`
- `src/system/IntentSystem.cpp`
- `src/system/LiquidSystem.cpp`
- `src/system/SupplySystem.cpp` *(new)*
- `src/system/SupplySystem.h` *(new)*
- `src/system/Systems.h`

**`f4658cc` — Merge: dima physics/input/order rework**

Reconciled supply feature with `dima` branch changes:
- `PhysicsContext` made instance-based; threaded through `supplyButtonSystem`/`spawnFalling` and the customer cycle (`customerSpawnerSystem`→`spawnCustomer`→`createCustomer`).
- Input model: supply buttons now flow through `intentSystem` (`updateSpawnButtonIntent`); drag guard moved into `updateDragIntent`. No more `SdlEvents` entity.
- Renamed `buttonSystem`→`supplyButtonSystem` (dima has its own `buttonSystem` for the coffee machine).
- Adopted array-based `Order` and rich order-icon display from dima/Jonathan; moved icon grid from `MainGameScene::onInit` into `spawnCustomer`. `randomDrinkOrder`→`randomOrder`.
- `debugDrawCupWalls`→`debugHighlightPhysics`.

**`56c114b` — Include cleanup post-merge**

Dropped includes that became dead after the merge:
- `src/MainGameScene.cpp` — removed `Menu.h`, `SpriteDims.h`
- `src/MainGameScene.h` — removed `Ingredient.h`, `WorldPos.h`, `bagel.h`
- `src/system/SupplySystem.cpp` — removed `<cmath>`

---

### C. Beverage check & grading pipeline — EitanVeryKatz

| Hash | Date | Subject |
|------|------|---------|
| `3c28b28` | 2026-06-16 | feat: add CheckCoffeeIntent and CheckBeverageSystem integration |
| `1670dc3` | 2026-06-16 | refactor: enhance coffee handling in CustomerSystem |
| `604b104` | 2026-06-16 | refactor: update drink grading logic to use CoffeeOverview |
| `3ef2fab` | 2026-06-16 | refactor: rename and enhance beverage grading system |
| `28ceda3` | 2026-06-16 | feat: implement checkBeverageSystem for beverage overview tracking |
| `1bc3440` | 2026-06-16 | refactor: streamline beverage assessment and update logic |
| `c6c748c` | 2026-06-16 | refactor: update CheckCoffeeIntent removal method for clarity |
| `8f98d5e` | 2026-06-16 | refactor: improve beverage management integration and clarity |

**`3c28b28` — Scaffold CheckCoffeeIntent + CheckBeverageSystem**

Introduced the two new source files and registered them in `CMakeLists.txt`. Wired `checkBeverageSystem` call into `MainGameScene.cpp` update loop.

Files changed:
- `src/CMakeLists.txt`
- `src/MainGameScene.cpp`
- `src/system/CheckBeverageSystem.cpp` *(new)*
- `src/system/CheckBeverageSystem.h` *(new)*

**`1670dc3` — Gate delivery on CheckCoffeeIntent; add CoffeeOverview**

`deliverySystem` now only serves coffee to customers that have a `CheckCoffeeIntent` component. Added `component/CoffeeOverview.h` to hold per-cup beverage state. Registered both in `Components.h` and `Systems.h`.

Files changed:
- `src/CMakeLists.txt`
- `src/component/CheckCoffeeIntent.h` *(new)*
- `src/component/CoffeeOverview.h` *(new)*
- `src/component/Components.h`
- `src/system/CustomerSystem.cpp`
- `src/system/Systems.h`

**`604b104` — gradeDrink takes CoffeeOverview instead of Cup**

`OrderMatch`: renamed `gradeDrinkRatio`→`gradeDrink`; changed parameter from `Cup` to `CoffeeOverview`; compares desired ratio against the overview's computed ratio. Updated call site in `CustomerSystem.cpp`.

Files changed:
- `src/OrderMatch.cpp`
- `src/OrderMatch.h`
- `src/component/CoffeeOverview.h`
- `src/system/CustomerSystem.cpp`

**`3ef2fab` — Rename to acceptGradedBeverageSystem; use CheckCoffeeIntent in grading**

Renamed `checkBeverageSystem`→`acceptGradedBeverageSystem`. Updated grading logic to use `CheckCoffeeIntent` for more accurate per-customer assessments. Adjusted `deliverySystem` integration. Touched: `OrderMatch.{h,cpp}`, `CheckCoffeeIntent.h`, `CustomerSystem.cpp`, `CheckBeverageSystem.{h,cpp}`, `MainGameScene.cpp`.

**`28ceda3` — Implement buildOverview; write CoffeeOverview per cup**

`CheckBeverageSystem.cpp`: added `buildOverview` function — iterates cup's liquid drops, accumulates ingredient counts, computes ratios and total drop count, writes result into `CoffeeOverview`. `checkBeverageSystem` calls `buildOverview` each update. `MainGameScene.cpp` wired to call it.

Files changed:
- `src/MainGameScene.cpp`
- `src/system/CheckBeverageSystem.cpp`
- `src/system/CheckBeverageSystem.h`

**`1bc3440` — Rename back to checkBeverageSystem; refine buildOverview**

Reverted rename to `checkBeverageSystem` for consistency with codebase naming patterns. Refined `buildOverview` ingredient ratio calculation. Added entries to `Menu.h`.

Files changed:
- `src/Menu.h`

**`c6c748c` — del\<\> not remove\<\>**

Changed `e.remove<CheckCoffeeIntent>()` → `e.del<CheckCoffeeIntent>()` to match bagel ECS API convention.

Files changed:
- `src/system/CheckBeverageSystem.cpp`

**`8f98d5e` — OrderMatch integration polish**

Refined method calls in `OrderMatch.cpp` for readability and consistency with updated delivery/beverage pipeline.

Files changed:
- `src/OrderMatch.cpp`

---

### D. Physics tuning & test cleanup — 1mache

| Hash | Date | Subject |
|------|------|---------|
| `477d8d5` | 2026-06-16 | Changed gravity and deleted old main |

Tuned gravity constant in `GameConfig.h`. Removed `app/sensor_input_test.cpp` (−299 lines) — isolated test entry point no longer needed.

Files changed:
- `src/GameConfig.h`
- `app/sensor_input_test.cpp` *(deleted)*

---

## Chronological Commit Table

| # | Hash | Date | Author | Subject | Key files |
|---|------|------|--------|---------|-----------|
| 1 | `477d8d5` | 2026-06-16 | 1mache | Changed gravity and deleted old main | `GameConfig.h`, `sensor_input_test.cpp` |
| 2 | `8f98d5e` | 2026-06-16 | EitanVeryKatz | refactor: improve beverage management integration | `OrderMatch.cpp` |
| 3 | `c6c748c` | 2026-06-16 | EitanVeryKatz | refactor: update CheckCoffeeIntent removal method | `CheckBeverageSystem.cpp` |
| 4 | `1bc3440` | 2026-06-16 | EitanVeryKatz | refactor: streamline beverage assessment | `Menu.h` |
| 5 | `28ceda3` | 2026-06-16 | EitanVeryKatz | feat: implement checkBeverageSystem + buildOverview | `CheckBeverageSystem.{h,cpp}`, `MainGameScene.cpp` |
| 6 | `3ef2fab` | 2026-06-16 | EitanVeryKatz | refactor: rename + enhance beverage grading | `OrderMatch`, `CheckCoffeeIntent.h`, `CustomerSystem.cpp` |
| 7 | `604b104` | 2026-06-16 | EitanVeryKatz | refactor: gradeDrink takes CoffeeOverview | `OrderMatch.{h,cpp}`, `CoffeeOverview.h`, `CustomerSystem.cpp` |
| 8 | `1670dc3` | 2026-06-16 | EitanVeryKatz | refactor: gate delivery on CheckCoffeeIntent | `CheckCoffeeIntent.h`, `CoffeeOverview.h`, `Components.h` |
| 9 | `3c28b28` | 2026-06-16 | EitanVeryKatz | feat: add CheckCoffeeIntent + CheckBeverageSystem | `CheckBeverageSystem.{h,cpp}`, `CMakeLists.txt` |
| 10 | `6ba91e0` | 2026-06-16 | EitanVeryKatz | Merge into EitanK | — |
| 11 | `56c114b` | 2026-06-15 | omerzv | chore: remove unused includes after dima merge | `MainGameScene.{h,cpp}`, `SupplySystem.cpp` |
| 12 | `f4658cc` | 2026-06-15 | omerzv | merge: integrate origin/dima into supply feature | `MainGameScene.cpp`, `Components.h`, `IntentSystem.cpp` |
| 13 | `e7e2f4f` | 2026-06-15 | omerzv | feat: button-summoned supply with drop-in | `SupplySystem.{h,cpp}`, `ButtonFactory.{h,cpp}`, `Falling.h`, `DeliveredTo.h`, `SpawnButton.h` |
| 14 | `8520115` | 2026-06-15 | 1mache | Added coffee machine collider | `CoffeeMachineFactory.cpp` |
| 15 | `e914a59` | 2026-06-15 | 1mache | Buttons work | `MachineButton.h`, `CoffeeMachineSystem.{h,cpp}`, `IntentSystem.cpp` |
| 16 | `605a2e7` | 2026-06-15 | 1mache | Buttons are drawn to the screen | `Button.h`, `CoffeeMachineFactory.cpp` |

---

## Files Touched — Rollup

### `src/component/`
| File | Stream(s) |
|------|-----------|
| `Button.h` *(created then deleted)* | A |
| `MachineButton.h` | A |
| `CheckCoffeeIntent.h` | C |
| `CoffeeOverview.h` | C |
| `DeliveredTo.h` | B |
| `Falling.h` | B |
| `SpawnButton.h` | B |
| `Liquid.h` | B |
| `Components.h` *(hotspot)* | A, B, C |

### `src/system/`
| File | Stream(s) |
|------|-----------|
| `CoffeeMachineSystem.{h,cpp}` | A |
| `CheckBeverageSystem.{h,cpp}` | C |
| `SupplySystem.{h,cpp}` | B |
| `CustomerSystem.cpp` *(hotspot)* | B, C |
| `IntentSystem.cpp` | A, B |
| `LiquidSystem.cpp` | B |
| `Systems.h` | A, B, C |

### `src/entity/`
| File | Stream(s) |
|------|-----------|
| `ButtonFactory.{h,cpp}` | B |
| `CoffeeMachineFactory.cpp` | A |
| `Entities.h` | B |

### `src/` root
| File | Stream(s) |
|------|-----------|
| `MainGameScene.cpp` *(hotspot)* | A, B, C |
| `MainGameScene.h` | B |
| `OrderMatch.cpp` *(hotspot)* | C |
| `OrderMatch.h` | C |
| `CMakeLists.txt` *(hotspot)* | A, B, C |
| `GameConfig.h` | D |
| `Menu.h` | C |
| `UserInput.h` | A |

### `app/`
| File | Stream(s) |
|------|-----------|
| `sensor_input_test.cpp` *(deleted)* | D |

---

## Notable Themes

**Beverage grading API churn.** `CheckBeverageSystem` was renamed `acceptGradedBeverageSystem` and then back to `checkBeverageSystem` across four commits. The `gradeDrink` function's parameter type moved from `Cup` to `CoffeeOverview` as the grading model clarified. API has now settled.

**PhysicsContext instance-based.** The merge commit (`f4658cc`) threaded `PhysicsContext` as an instance through the supply and customer spawning paths. Aligns with the roadmap goal to eliminate the global `PhysicsContext`.

**Intent-based input adoption.** Supply buttons now route through `updateSpawnButtonIntent` in `intentSystem` rather than reading SDL events directly. Consistent with the `code-conventions.md` InputSystem goal where all input state is exposed via components.

**Hotspot files.** `MainGameScene.cpp`, `CustomerSystem.cpp`, `OrderMatch.{h,cpp}`, `CMakeLists.txt`, and `Components.h` were each touched by two or more independent streams — highest merge-conflict risk going forward.
