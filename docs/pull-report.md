# Pastry TV — How It Works, and Best-Practices Check

Commit `6096a79` (Jonathan): "Add Pastry TV system: Implement PastryTv and PastryTvSystem to cycle through pastry types and integrate with supply system."

## What it is

Top-left decorative screen (`supply::PASTRY_TV_POS`) that cycles through the 5 `PastryType` values every `PASTRY_TV_ROTATE_TIME` (5s), looping. The pastry spawn button always spawns whatever the TV is currently showing.

## Files

| File | Role |
|---|---|
| `src/component/PastryTv.h` | `PastryTv{ index, timer }` — lives on the pastry-icon entity |
| `src/entity/PastryTvFactory.cpp` | `createPastryTv` — spawns TV frame (static) + pastry-icon entity (`PastryTv` + `Drawable`) |
| `src/system/PastryTvSystem.cpp` | `pastryTvSystem` (advances `index`/`timer`, rewrites `Drawable.srcRect`) + `currentTvPastryType()` |
| `src/system/SupplySystem.cpp` | `supplyButtonSystem` — spawns pastry via `createPastry(..., currentTvPastryType())` |
| `src/entity/PastryFactory.cpp` | `createPastry` — if `type == PastryType::count`, picks random; else uses given type |

## Entity setup (`createPastryTv`)

Two entities, both at `pos`:
1. **TV frame** — `Transform + Drawable` only, static decoration, layer `STATIC_ON_BARTOP`.
2. **Pastry display** — `Transform + Drawable + PastryTv{}`, layer `STATIC_OVERLAY`, initial sprite = first frame of the `"pastry"` sprite-sheet tag.

## Per-frame flow (`MainGameScene::onUpdate`)

```
intentSystem            → sets SpawnButton.justPressed on click
pastryTvSystem(dt)       → advance TV BEFORE it's read this frame
supplyButtonSystem       → reads currentTvPastryType(), spawns pastry
```

**`pastryTvSystem`**: for every entity with `PastryTv + Drawable`, adds `dt` to `timer`. Past `PASTRY_TV_ROTATE_TIME`, subtracts the interval, `index = (index + 1) % PastryType::count`, and directly overwrites `Drawable.srcRect` with the new frame (`pastryFrom + index`, where `pastryFrom` = start of the `"pastry"` tag in `props.json`, currently 0).

**`currentTvPastryType()`**: separately loops all entities for the one with `PastryTv`, returns `static_cast<PastryType>(index)`. Returns `PastryType::count` (sentinel) if no TV exists.

**Spawn ("how does it know which to spawn")**: `supplyButtonSystem` — on a `SpawnButton` with `item == DropType::Pastry` and `justPressed`, and no entity blocking the spawn slot — calls `createPastry(assets, physics, spawnPos, currentTvPastryType())`. `createPastry` uses the given type as-is (frame index = `static_cast<int>(type)` directly into `props.png`, not offset by any tag). So: **TV index → `currentTvPastryType()` → spawn button reads it directly → pastry created with that exact type.** No randomness once a TV exists.

## Against `docs/code-conventions.md`

**Violation — "Systems Communicate Through Components."**
`supplyButtonSystem` (in `SupplySystem.cpp`) calls `currentTvPastryType()`, a function defined in `PastryTvSystem.cpp`, directly. That's one system reaching into another system's internals — exactly what the doc forbids:

> "Systems must not call into each other. If system A needs to affect system B's behavior, A writes a component — B reads it next frame."

The component to read already exists (`PastryTv.index` on the TV entity) — `supplyButtonSystem` could query for the `PastryTv` entity itself, or `pastryTvSystem` could publish the current type onto a small shared/global-lookup component instead of exposing a free function. As written, `currentTvPastryType()` re-does the same `bagel::Entity::first()...` mask-scan `pastryTvSystem` already does, once per pastry-button press — redundant and coupled.

**Fragile (not a doc violation, but worth flagging):** `PastryFactory::getSpriteFromType` indexes `props.png` with `static_cast<int>(type)` directly (comment: "Value == its props.png frame index"), while `pastryTvSystem` indexes with `pastryFrom + tv.index` (offset from the `"pastry"` frameTag, currently `0` in `props.json`). The two only agree because the tag happens to start at frame 0 today. If the sprite sheet is ever re-tagged so `"pastry"` doesn't start at index 0, the TV's displayed icon and the actually-spawned pastry type silently diverge. No shared constant/assert ties these two indexing schemes together.

**Compliant:**
- Render layers: both TV entities set an explicit layer (`STATIC_ON_BARTOP`, `STATIC_OVERLAY`) — no implicit z-order.
- No direct SDL event polling — button press arrives via `SpawnButton.justPressed`, set upstream by `intentSystem`.
- No new globals introduced.
- `supplyButtonSystem(AssetManager&, PhysicsContext&)` — correct `AssetManager` before `PhysicsContext` order.
- Files placed per `project-structure.md`: component in `component/`, factory in `entity/*Factory.{h,cpp}`, system in `system/`, declarations added to `Components.h`/`Entities.h`/`Systems.h` umbrellas.
