# Project Structure — Code Organization Guide

## Folder layout

```
cafe-ecs/
  app/               # entry points (main.cpp and isolated test mains)
  src/
    component/       # one file per component + its Storage specialization
    system/          # system files grouped by cohesion
    entity/          # one *Factory file per entity type
    (root)           # contexts, config, assets, utilities
  tests/             # Catch2 unit tests
  docs/              # team docs (you are here)
  dependencies/      # vendored libraries (bagel, SDL3, box2d, …)
  res/               # runtime assets (textures, json)
```

---

## `src/component/` — one file per component

Each component gets its own header. No `.cpp` unless it has non-trivial free functions (only `Transform.cpp` currently exists).

**What goes in the header:**
1. The `struct` (data only — no methods, no behavior).
2. Its `bagel::Storage<>` specialization, at global scope after the `namespace cafe` close.
3. Any closely related free functions whose *only* job is to convert or inspect that component's data (e.g. `worldToScreenPoint`, `transformToFrect` live in `Transform.h`).

```
component/
  Transform.h / .cpp     # Transform struct + conversion helpers
  Drawable.h             # struct + Storage
  ChildOf.h              # struct + Storage
  Liquid.h               # tag + Storage
  PhysicsBody.h          # struct + Storage
  Cup.h                  # struct + Storage
  CoffeeSpawner.h        # struct + Storage
  CleanupZone.h          # tag + Storage
  Order.h                # INGREDIENT_KINDS + Ingredient enum + Order struct + Storage
  Behavior.h             # struct + Storage
  Leaving.h              # tag + TaggedStorage  ← note: tag = TaggedStorage, not Sparse
  SpeechAnchor.h         # struct + Storage
  Components.h           # umbrella: #includes all of the above
```

**Storage kind rule:**
- Regular data struct → `SparseStorage<T>`
- Tag (empty struct, presence-only) → `TaggedStorage<T>`

**Umbrella:** `#include "Components.h"` gets everything. Individual headers also resolve as `#include "Transform.h"` (all three subfolders are on the include path).

---

## `src/system/` — grouped by cohesion

Systems that share file-local state or operate on the same feature area live in the same file. One system per file is also fine if it stands alone.

```
system/
  RenderSystem.h / .cpp      # drawSystem
  TransformSystem.h / .cpp   # syncTransformFromBody, hierarchySystem
  CoffeeSystem.h / .cpp      # coffeeSpawnerSystem, sensorEventSystem, dumpDebugStatsEvery
  ClientSystem.h / .cpp      # behaviorSystem, orderSystem, cleanupSystem
  Systems.h                  # umbrella: #includes all system headers
```

**When to group:** if two systems share a `static` file-local variable (e.g. `g_stats` in CoffeeSystem) or are always called together as one logical unit, put them in the same `.cpp`.

**When to split:** if a system grows large enough that its `.cpp` is hard to navigate, or if it starts handling a clearly separate concern, give it its own file.

**Systems must not call each other.** Communication only through components — see `CLAUDE.md`.

---

## `src/entity/` — one `*Factory` file per entity type

Each factory gets its own `*Factory.{h,cpp}`. The `*Factory` suffix avoids basename collisions with same-named component headers (`Cup.h` is a component; `CupFactory.h` is the entity).

```
entity/
  CupFactory.h / .cpp              # createCup, createCupHeadless
  LiquidDropFactory.h / .cpp       # createLiquidDrop
  CoffeeMachineFactory.h / .cpp    # createCoffeeMachine
  CleanupZoneFactory.h / .cpp      # createCleanupZone
  ClientFactory.h / .cpp           # createClient
  SpeechBubbleFactory.h / .cpp     # createSpeechBubble
  OrderIconFactory.h / .cpp        # createOrderIcon
  Entities.h / .cpp                # umbrella + destroyPhysicalEntity
```

**What a factory does:** creates a `bagel::Entity`, calls `ent.addAll(...)` with the right components, returns the entity. No game logic — just construction.

**File-local helpers** (e.g. `createCupCommon` in `CupFactory.cpp`) are fine. Keep them in the same `.cpp` as the public factory they serve; never put them in a header.

---

## `src/` root — everything else

Contexts, config, asset management, utilities. These are not ECS components or systems; they are support infrastructure.

```
GameConfig.h       # compile-time constants (PTM, LOGICAL_W/H, …)
WorldPos.h         # WorldPos alias / helper
RenderContext.h    # SDL_Renderer + SDL_Window singleton
PhysicsContext.h/cpp  # b2WorldId singleton + step
PhysicsFilters.h   # Box2D collision category/mask constants
Assets.h/cpp       # named texture accessors (static singleton)
AssetManager.h/cpp # path-keyed texture cache (instance-based)
Texture.h/cpp      # RAII SDL_Texture wrapper
Utils.h            # assertFatal and other small utilities
```

---

## `app/` — entry points

`main.cpp` is the game entry point. For testing an isolated feature, add a dedicated `my_feature.cpp` with its own `main()` and register it in `app/CMakeLists.txt`. Remove test entry points before merging to `main`. See `CLAUDE.md` for the exact CMake snippet.

---

## Adding new code — decision tree

```
Is it a data-only struct that entities carry?
  → src/component/MyThing.h  (+ Storage specialization)
  → add to component/Components.h

Is it a function that transforms/reads only that one component's data?
  → same header as the component

Is it game logic that runs every frame over a set of entities?
  → src/system/  (new file, or add to an existing cohesive group)
  → add declaration to system/Systems.h

Is it a function that constructs a specific entity from components?
  → src/entity/MyThingFactory.{h,cpp}
  → add to entity/Entities.h

Is it shared infrastructure (context, config, RAII wrapper, utility)?
  → src/ root
```
