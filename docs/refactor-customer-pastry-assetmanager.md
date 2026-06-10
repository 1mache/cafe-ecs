# Refactor: Customer, Pastry, and AssetManager-First Factories

Covers commits `9439c3a` through `484dbb8` (since `be3201c`).

---

## Summary

Six commits that together:
1. Renamed `Client` → `Customer` throughout.
2. Introduced draggable `Pastry` entity.
3. Refactored all entity factories to take `AssetManager&` instead of raw `SDL_Texture*`.
4. Split `Cup` into back/front child entities for correct liquid layering.
5. Added `Served` component and wired delivery → order completion.
6. Exposed game state (`_machineEnt`) to the event loop for SPACE-to-pour.

---

## 1. Client → Customer Rename

`ClientFactory` and `ClientSystem` renamed to `CustomerFactory` / `CustomerSystem`. No logic changes; pure semantic clarity.

Affected files:
- `src/entity/ClientFactory.*` deleted → `src/entity/CustomerFactory.*`
- `src/system/ClientSystem.*` → `src/system/CustomerSystem.*`
- `reportLeavingClients()` → `reportLeavingCustomers()`

---

## 2. AssetManager-First Factories

All factories previously received raw `SDL_Texture*` + explicit `texW`/`texH` floats from the caller. Now every factory that needs a texture takes `AssetManager& assets` and resolves textures internally via a file-local constant:

```cpp
// Before (in CafeGame::run)
auto& machineTex = getAssetManager().getTexture(TEX_MACHINE);
auto machineEnt  = createCoffeeMachine({-4.f, 1.f}, {0.f, -0.5f},
                                        machineTex.get(), texW, texH);

// After (in CafeGame::init)
_machineEnt = createCoffeeMachine(assets, {-6.f, -1.f}, {0.f, -0.5f});
```

`TEX_*` string constants removed from `CafeGame.h`; each factory owns its own `static constexpr auto TEX = "file.png"` locally.

Affected factories: `CoffeeMachineFactory`, `CupFactory`, `CustomerFactory`, `LiquidDropFactory`, `OrderIconFactory`, `SpeechBubbleFactory`, `CafeEnvironmentFactory`.

**Headless variants** (`createCupHeadless`, `createLiquidDropHeadless`) kept for tests — pass `nullptr` texture, skip asset loading.

---

## 3. Pastry Entity

New `PastryFactory` creates a kinematic draggable pastry on the counter.

Components:
| Component | Value |
|---|---|
| `Transform` | position + half-extents from `PROP_DIMS` (16×16 px) |
| `Drawable` | `props.png` frame 0 (cinnamon roll), `LAYER_PROP` |
| `PhysicsBody` | kinematic Box2D body |
| `Draggable` | `DropType::Any` |

Also calls `addDraggableVisitorShape()` to register a sensor shape for drag hit-testing.

---

## 4. Cup Split into Back/Front Entities

`createCupCommon()` now creates **two entities** sharing a transform origin:

| Entity | Components | Render Layer |
|---|---|---|
| `cupBack` | Transform, Drawable (back frame), PhysicsBody, Cup, Draggable | `LAYER_CONTAINER_BACK` |
| `cupFront` | Transform, Drawable (front frame), ChildOf(cupBack) | `LAYER_CONTAINER_FRONT` |

Liquid fill (rendered by `drawCupLiquid()`) sits between the two layers, giving the illusion of liquid inside the cup without manual draw-order hacks.

The physics body (3 solid walls + 1 interior sensor) lives on `cupBack`. `cupFront` is visual only.

---

## 5. Customer Delivery: Served Component

`makeCustomerDeliverable()` is called during `createCustomer()` and attaches:

- A **static Box2D body** with a `DROPSPACE_SENSOR` shape sized to the customer's transform.
- `DropSpace{ DropType::Any }` — marks the entity as a valid drop target.
- `Served{}` — tracks what the customer has received.

```cpp
struct Served {
    bool drink{};
    bool pastry{};
};
```

`deliverySystem()` sets `served.drink = true` or `served.pastry = true` when an item is dropped on the customer. `orderSystem()` checks whether both required items (per `Order::hasDrink`, `Order::hasPastry`) are satisfied, then marks the customer `Leaving` with `rating = 1`.

---

## 6. Ingredient Rename: Tea → Water

`Order::ratio` third slot renamed from `Tea` to `Water`. Enum updated:

```cpp
// Before
enum class Ingredient { Coffee, Milk, Tea };

// After
enum class Ingredient { Coffee = 0, Milk, Water, count };
static constexpr size_t INGREDIENT_COUNT = static_cast<int>(Ingredient::count);
```

`INGREDIENT_KINDS` constant removed; use `INGREDIENT_COUNT` instead.

---

## 7. CafeGame Restructure

Entity creation moved from `run()` into `init()`. `run()` is now event loop + system tick only.

`_machineEnt` stored as a member to avoid per-frame ECS queries for SPACE-to-pour:

```cpp
bagel::Entity _machineEnt{static_cast<bagel::ent_type>(-1)};
```

Startup entity creation order in `init()`:
1. Background
2. Bartop
3. Coffee machine → stored in `_machineEnt`
4. Cup (capacity 50)
5. Pastry
6. Cleanup zone
7. Customer (60 s patience, order with drink + pastry)
8. Speech bubble (child of customer)
9. Order icons (children of bubble)

Game loop system order per frame:
1. `coffeeSpawnerSystem` — spawn drops
2. `PhysicsContext::step` — advance physics
3. `sensorEventSystem` — count drops into cup; destroy spilled
4. `dropSpaceDetectionSystem` — update `Held.dropSpaceEntity`
5. `syncTransformFromBody` — physics → Transform
6. `behaviorSystem` — tick patience; add Leaving on timeout
7. `orderSystem` — full cup + pastry → rating=1 + Leaving
8. `reportLeavingCustomers` — log result
9. `hierarchySystem` — children follow parents; orphan Leaving children
10. `cleanupSystem` — destroy Leaving entities
11. Render

---

## 8. New / Removed Files

| Status | File |
|---|---|
| Added | `src/entity/CustomerFactory.h/.cpp` |
| Added | `src/entity/PastryFactory.h/.cpp` |
| Added | `src/component/Served.h` |
| Added | `src/component/Draggable.cpp` |
| Added | `src/system/InputSystem.h` |
| Added | `src/SpriteDims.h` |
| Deleted | `src/entity/ClientFactory.h/.cpp` |
| Deleted | `src/component/SpeechAnchor.h` |
| Deleted | `src/components/` (entire directory — duplicate headers) |
| Renamed | `ClientSystem.*` → `CustomerSystem.*` |

`src/components/` was a stale duplicate of `src/component/`; all its headers (`Behavior`, `ChildOf`, `Drawable`, `Leaving`, `Order`, `SpeechAnchor`, `Transform`) were consolidated under `src/component/`.

---

## 9. Render Tinting for Liquid Drops

`RenderSystem` now tints `Liquid`-tagged entities to coffee color before drawing:

```cpp
if (e.has<Liquid>())
    SDL_SetTextureColorMod(d.texture, kCoffeeR, kCoffeeG, kCoffeeB);  // ~#4B2F1E
SDL_RenderTexture(renderer, d.texture, &d.srcRect, &dstRect);
if (e.has<Liquid>())
    SDL_SetTextureColorMod(d.texture, 255, 255, 255);  // reset
```

Particle sprite stays white in the asset; color is applied at draw time.
