# Coffee Fluid Particles — Team Cheat Sheet

**Owner:** Omer
**Branch:** `omer`
**What this is:** Coffee drops spawned from the machine, falling under gravity, caught (and counted) by the cup, cleaned up if they spill. Plus the Box2D foundation the rest of the game can use for physics.

---

## TL;DR

- Hold **SPACE** to pour coffee from the machine. Release to stop.
- The cup catches drops, counts them in `Cup::filled`, and destroys each drop on contact.
- Drops that miss the cup are destroyed by an off-screen cleanup sensor.
- All physics goes through one shared `b2World` (in `PhysicsContext`).
- **Collision filtering is in place so this won't interfere with the drag-and-drop dropspace feature** — see "If you're building dropspace" below.

---

## Files added / changed

| File | Purpose |
|---|---|
| `src/PhysicsContext.h/.cpp` | Owns the Box2D world (`b2World`). One `init` / `step(dt)` / `shutdown`. |
| `src/PhysicsFilters.h` | Collision-category bits + masks. The contract file between teammates. |
| `src/Assets.h/.cpp` | Loads & caches shared textures (`particle.png`, `cup.png`, `machine.png`). |
| `src/Components.h` | Added 5 components: `Liquid`, `PhysicsBody`, `Cup`, `CoffeeSpawner`, `CleanupZone`. |
| `src/Entities.h/.cpp` | Added 5 factories + `destroyPhysicalEntity`. |
| `src/Systems.h/.cpp` | Added 3 systems: spawner, sensor events, body→transform sync. |
| `app/sdl_main.cpp` | Wired everything into the game loop. Temporary SPACE-key trigger. |
| `dependencies/bagel/bagel.h` | Bumped `MaxComponents` from 6 to 32 (headroom for everyone). |
| `tests/test_*.cpp` | 7 test cases across filters / spawner / end-to-end fill. |

---

## New ECS components

```cpp
struct Liquid {};                  // tag — "this is a coffee drop"
struct PhysicsBody { b2BodyId id; };   // any entity that exists in Box2D
struct Cup { int capacity, filled; float fillPercent(); };
struct CoffeeSpawner { float interval; float accumulator; bool active; WorldPos offset; };
struct CleanupZone {};             // tag — "this is the off-screen kill sensor"
```

Each one has its `bagel::Storage<...>` specialization already wired up — just `addAll(...)` them when creating entities.

---

## How to use it

### Spawn a machine and a cup
```cpp
auto machine = createCoffeeMachine({ -4.f, 1.f }, { 0.f, -0.5f });
auto cup     = createCup         ({ -4.f, -1.f }, /*capacity=*/ 50);
createCleanupZone();
```
World coords are in **meters, Y-up**. `{0,0}` is screen center. PTM=8 means 8 px per meter.

### Start / stop pouring
```cpp
machine.get<CoffeeSpawner>().active = true;   // pour
machine.get<CoffeeSpawner>().active = false;  // stop
```

### Read fill state
```cpp
auto& c = cup.get<Cup>();
c.filled;          // int — drops caught
c.capacity;        // int — max
c.fillPercent();   // 0.0 .. 1.0 — useful for a progress bar
```

### Destroying physical entities
```cpp
destroyPhysicalEntity(cup.entity());   // ✅
cup.destroy();                          // ❌ leaks the b2Body
```
**Always** use `destroyPhysicalEntity` for anything that has a `PhysicsBody`. If you `entity.destroy()` directly, the Box2D body is orphaned and the next frame may crash.

---

## If you're building **dropspace** (drag-and-drop sensors)

This is the contract that lets us not break each other. Read `src/PhysicsFilters.h`.

**Bits I own:** 0–3 (`LIQUID`, `CUP_SOLID`, `CUP_INSIDE`, `CLEANUP`).
**Bits reserved for you:** 4–7. Pick whatever names you like — `DROPSPACE_SENSOR`, `DRAGGABLE`, etc.
**Bits 8+:** unassigned. Coordinate before claiming.

For any sensor / fixture you create:
```cpp
b2ShapeDef sd = b2DefaultShapeDef();
sd.isSensor = true;                 // if it's a sensor
sd.enableSensorEvents = true;       // IMPORTANT — Box2D 3.x defaults to false
sd.filter.categoryBits = filter::YOUR_BIT;
sd.filter.maskBits     = filter::YOUR_MASK;
b2CreatePolygonShape(body, &sd, &shape);
```

**Two rules that keep us decoupled:**
1. Your mask should NOT contain bit 0 (`LIQUID`). Coffee drops shouldn't trigger your dropspace.
2. My `MASK_LIQUID` already excludes bits 4–7, so even if you forget rule 1 by accident, Box2D's two-way filter check still blocks the contact. We're double-safe.

**To add a new category to the registry:** edit `PhysicsFilters.h` directly and add your bits + masks alongside mine. Don't put magic numbers anywhere else.

---

## Adding physics to entities you create

If you create an entity that has a Box2D body (a customer with a hitbox, a pastry that can be picked up, etc.):

1. **Store the entity id in the body's user-data** so sensor events can find your entity back:
   ```cpp
   bd.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
   ```
2. **Add the `PhysicsBody{ body }` component** so the body→Transform sync system picks it up.
3. **Set sensor flags correctly** (see above).
4. **Use `destroyPhysicalEntity`** when killing the entity later.

---

## The game-loop order (don't reorder)

In `app/sdl_main.cpp`:
```cpp
coffeeSpawnerSystem(dt);   // 1. spawn new drops
PhysicsContext::step(dt);  // 2. advance physics
sensorEventSystem();       // 3. handle CUP_INSIDE / CLEANUP events
syncTransformFromBody();   // 4. copy b2Body pos into Transform
drawSystem();              // 5. render
```
- Spawn before step so new drops participate immediately.
- Sensor handling after step (events are produced by the step).
- Transform sync after sensor handling (don't waste work on destroyed entities).

---

## Tunable knobs

| Knob | File | Default | Meaning |
|---|---|---|---|
| `GRAVITY` | `GameConfig.h:18` | `25.f` | Downward acceleration (m/s²) |
| `FPS` | `GameConfig.h:5` | `60` | Frame target + physics fixed step |
| `PTM` | `GameConfig.h:7` | `8.f` | Pixels per meter |
| Drop radius `r` | `Entities.cpp` (`createLiquidDrop`) | `0.06f` | Raise to `0.10f` if drops jitter |
| `interval` | `CoffeeSpawner` defaults / per-instance | `0.05f` | Seconds between drops |
| Cup `capacity` | 2nd arg to `createCup` | `50` | Drops to "full" |
| Wall thickness `wallT` | `Entities.cpp` (`createCupCommon`) | `0.5f / PTM` | Cup wall thickness in meters |
| Spout offset | 2nd arg to `createCoffeeMachine` | `(0, -0.5)` | Where drops appear |

---

## Tests

```bash
ctest --preset core-test-windows
```
7 cases pass:
- 3× filter-mask invariants (compile-time `STATIC_REQUIRE`)
- 3× spawner timing (active, inactive, offset)
- 1× end-to-end fill (drop falls into cup, counter increments, drop destroyed)

The fill test stands up the real `PhysicsContext` and runs 60 fixed steps — no SDL window needed.

---

## Gotchas worth knowing

1. **`enableSensorEvents` defaults to `false`** in Box2D 3.x. Both sides of a sensor contact must opt in. The migration docs are misleading on this.
2. **`b2ShapeDef::friction` doesn't exist** — Box2D 3.x moved it to `b2ShapeDef::material::friction`.
3. **Drops are bullets** (`isBullet = true`) so they don't tunnel through the thin cup walls. If you make a similarly small/fast body, do the same.
4. **`MaxComponents` is now 32.** If you add components and run out of room, raise it more — bagel's mask type widens automatically.
5. **The cup is kinematic, not static.** That's intentional so dropspace can move it via `b2Body_SetTransform` later.
6. **Cleanup zone has no Transform and no Drawable.** It's invisible and immobile. Don't expect it in `drawSystem`.

---

## Known TODOs / future work

- **SPACE key is a temporary trigger.** Replace with a game-rule activation (e.g., cursor over machine + active order). Look for `TODO(team)` in `sdl_main.cpp`.
- **Fill visualization.** `Cup::filled` is a number; rendering a rising liquid surface inside the cup is not yet implemented.
- **Order/customer integration.** When a cup reaches capacity, somebody needs to decide what happens (serve, reset, destroy). That's the order system's call — `Cup::filled / Cup::capacity` is the API they read.
- **Cup placement.** Currently hardcoded to `{ -4.f, -1.f }` in `sdl_main.cpp` so it sits under the machine spout for the demo. When dropspace lands, this will be user-controlled.
- **No customer entity in `main` right now.** It was removed from `sdl_main.cpp` because it conflicted with the new system loop. Jonathan's branch (`origin/Jonathan`) will re-introduce it properly with the `Order`/`Behavior` components.

---

## Quick reference: which file to open when

- "I need to change a physics constant" → `src/GameConfig.h`
- "I need to claim a collision bit" → `src/PhysicsFilters.h`
- "I want to add a new component" → `src/Components.h` (don't forget the `bagel::Storage` specialization)
- "I want to add a new entity factory" → `src/Entities.h`/`.cpp`
- "I want to add a new system" → `src/Systems.h`/`.cpp`, then call it from the loop in `app/sdl_main.cpp`
- "I want to add a new sprite" → `res/`, then add it to `Assets.h`/`.cpp`
- "I'm debugging weird collision behavior" → check `enableSensorEvents`, then filter bits, then `userData`
