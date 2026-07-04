# Generalized Tween System — Design

Design-only report. No code changed. Answers: can napkin's intent+system pattern
generalize into a shared Transform tween, reused for customer walk-in/out?

---

## 1. Current napkin system (recap)

`NapkinIntent{ state }` (Hidden/Toggle/Full) is set by `IntentSystem` from mouse
hover/click on screen-space hitboxes. `napkinSystem` reads the intent each frame and
drives two independent easings toward `layoutForState(state)`:

- **Position**: `chaseTarget()` sets Box2D linear velocity on napkin's own
  **dynamic** `PhysicsBody` — speed scales with distance (capped), snaps to zero
  inside a small threshold. Box2D integrates the position during `_physics.step()`;
  `physicsToTransformSystem` copies it back into `Transform` afterward.
- **Size**: `easeSize()` exponential-lerps `Transform.w/h` directly, independent of
  physics.

Napkin's `PhysicsBody` exists *only* to borrow Box2D's velocity integration as a
smoothing mechanism — it has no collision role (hitboxes are plain screen-space
rect tests in `IntentSystem`). This is a reasonable hack, but it's a physics
dependency for what is conceptually a pure UI tween.

## 2. Proposed generic primitive

A single reusable component + system, no Box2D involvement:

```cpp
struct Tween
{
    Transform from;
    Transform to;
    float     duration;
    float     elapsed{0.f};
};
```

```cpp
void tweenSystem(float dt)
{
    // for each entity with Tween + Transform:
    //   elapsed += dt
    //   t = clamp(elapsed / duration, 0, 1)
    //   eased = smoothstep(t)              // t*t*(3-2t)
    //   Transform = lerp(from, to, eased)  // x, y, w, h all interpolated
    //   if t >= 1: remove Tween component  // presence of Tween == "in flight"
}
```

Key properties:

- **Presence-as-state**: `has<Tween>()` means "currently animating." Removal on
  completion is the arrival signal — matches existing bagel idioms (e.g.
  `DragIntentType` transitions) rather than adding a separate "arrived" flag.
- **Retargeting mid-flight** (needed for napkin, e.g. hover-away before Toggle
  finishes animating in): when the desired target changes, overwrite `from` with
  the entity's *current* (already-interpolated) `Transform`, set `to` to the new
  target, reset `elapsed = 0`, and pick a fresh `duration`. No velocity math, no
  snapping — the new leg simply starts from wherever the old one left off.
- **Duration, not speed**: fixed-duration was chosen for predictable walk pacing.
  Napkin's current "speed-capped chase" feel is preserved by *deriving* duration
  from distance at retarget time (`duration = distance / tunedSpeed`), rather than
  hardcoding a flat duration for every leg. Net effect on napkin should be close to
  current behavior; exact constant needs tuning by feel, not by formula.
- **No Box2D dependency**: entities that are pure UI/visual movers (napkin,
  customer sprite) don't need a `PhysicsBody` for this. Entities that *also* need a
  physics body for unrelated reasons (e.g. customer's dropspace sensor) attach/
  detach that body separately, gated by tween completion — see §4.

## 3. Napkin migration

- Drop `PhysicsBody` from `createNapkin` entirely — no dynamic body needed once
  movement is a pure Transform tween.
- `napkinSystem` collapses to: on `NapkinIntent.state` change, start/retarget a
  `Tween` toward `layoutForState(newState)` (full Transform: x, y, w, h together,
  replacing the separate position/size easings with one interpolation).
- `tweenSystem` handles the actual per-frame interpolation; `napkinSystem` only
  decides *when* a new target is needed.
- Removes napkin's entry from physics stepping/sync entirely — one less entity
  Box2D has to carry every frame.

## 4. Customer walk-in / walk-out

### New data

`CustomerSpawner` gains two more `WorldPos` fields alongside the existing `seat`:

```cpp
struct CustomerSpawner
{
    WorldPos seat;
    WorldPos entrance;  // off right edge — walk-in start
    WorldPos exit;      // off left edge — walk-out end
    float    interval;
    float    cooldown;
};
```

(Matches the earlier decision: customers enter from the right, leave to the left —
no sprite flip needed, so `Tween` only ever needs to move `x`, not touch facing.)

### State

```cpp
enum class CustomerWalkState { Arriving, Seated, Departing };

struct CustomerWalk
{
    WorldPos        exit;   // stashed at spawn time for the departure leg
    CustomerWalkState state;
};
```

### Flow

1. **Spawn** (`customerSpawnerSystem`): create customer at `entrance` (not `seat`).
   Speech bubble + order icons attach immediately (visible while walking in — lets
   the player read the order early). Add `CustomerWalk{ exit, Arriving }` +
   `Tween{ from: entrance, to: seat, duration }`. **Do not** call
   `makeCustomerDeliverable` yet — no dropspace sensor, no `PhysicsBody`, no
   `OrderGrade` until arrival.
2. **Arrival** (`customerWalkSystem`, new): `state == Arriving && !has<Tween>()` →
   call `makeCustomerDeliverable` (creates the static sensor body at the now-final
   seat `Transform`), set `state = Seated`.
3. **Leaving** (`customerWalkSystem`): `has<Leaving>() && state == Seated` → strip
   `PhysicsBody` / `DropSpace` / `OrderGrade` (no longer deliverable while walking
   out), set `state = Departing`, start `Tween{ from: seat, to: exit, duration }`.
   This is also the single edge where "customer left" should be logged (see next).
4. **Cleanup** (`customerCleanupSystem`, updated mask): destroy only when
   `has<Leaving>() && state == Departing && !has<Tween>()` — i.e. the walk-out
   animation has actually finished. Today it destroys on `Leaving` alone, same
   frame.

### Why this avoids the physics-sync conflict

`physicsToTransformSystem` runs every frame for any entity with `Transform` +
`PhysicsBody`, overwriting `Transform.x/y` from the body's position. Since the
dropspace sensor's static body doesn't exist until step 2 above, there's no body to
fight the tween during the walk-in leg. During walk-out (step 3), the body is
removed *before* the departure `Tween` starts, so the same non-conflict holds. This
was the deciding factor for gating deliverability on arrival rather than attaching
the sensor at spawn.

### Side-effect: `reportLeavingCustomers` needs to move

Today, `Leaving` appears and the entity is destroyed same frame, so the per-frame
`std::cout` log in `reportLeavingCustomers` fires exactly once per customer. Once
destruction is delayed until the walk-out tween finishes (roughly a
duration-worth of frames), that system would log every frame the customer is
mid-exit — spammy. Fix: move the log to the single-edge transition in step 3
(`customerWalkSystem`, right where `state` flips to `Departing`) instead of a
standalone per-frame system. `finalizeOrderGradeSystem` (also per-frame on
`Leaving`) is idempotent (recomputes the same rating each call) so it's harmless to
leave as-is, but could be tightened the same way later if it matters.

## 5. System ordering (MainGameScene::onUpdate)

`tweenSystem(dt)` is pure-Transform, no physics — it can run anywhere before
`drawSystem`. Suggested placement: alongside `napkinSystem`'s old slot (before
`_physics.step`), since neither napkin nor customer-walk cares about physics
ordering anymore:

```
napkinSystem(dt)          // becomes: decide targets only
customerWalkSystem(dt)    // new: decide targets, gate dropspace attach/detach, cleanup edge
tweenSystem(dt)           // new: shared interpolation for both
...
_physics.step(dt)
...
physicsToTransformSystem()   // unaffected; still needed for dropspace-sensor-bearing entities once seated (static body doesn't move, but keeps existing entities with PhysicsBody consistent)
...
customerCleanupSystem()      // mask updated per §4 step 4
```

## 6. Open items (need tuning, not decisions)

- Exact `duration` / derived-speed constants for napkin legs and customer walk —
  by feel, not calculable up front.
- Whether `entrance`/`exit` should be screen-relative constants or explicit
  per-spawner `WorldPos` (design above assumes the latter since there's currently
  only one `CustomerSpawner`, but the field lives on the component either way so
  multi-seat support is free).
- `smoothstep` vs. a different easing curve — arbitrary choice above, swappable
  without touching the component shape.
