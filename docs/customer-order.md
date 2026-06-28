# Customer Order System

How random orders are created, displayed in the speech bubble, and linked to the customer entity.

---

## 1. Order generation — `randomOrder()` (`src/Menu.cpp`)

Called by `customerSpawnerSystem` when the seat is empty and the cooldown expires. Returns an `Order` struct.

```
budget = MAX_ORDER_ICONS   // 5

wantDrinks   = randInt(0..MAX_DRINKS)    // 0–3
wantPastries = randInt(0..MAX_PASTRIES)  // 0–3
// invariant: at least one item — if both 0, flip a coin and force one
```

**Drinks loop** — for each of the `wantDrinks` slots:
1. Pick a random `ItemTypes` (Black, Espresso, Americano, Latte, Cappuccino).
2. Pick temperature: if the recipe allows both, flip a coin; otherwise forced.
3. Compute icon cost: `Cold drink → 2` (needs a second ice icon), `Hot drink → 1`.
4. If `cost > budget`, stop. Otherwise append to `order.drinks[]` and subtract cost.

**Pastries loop** — same pattern:
1. Pick a random `PastryType` (Croissant, CinnamonRoll, Bourekas, Cheesecake, CarrotCake).
2. Flip a coin for temperature (pastries allow both).
3. Compute icon cost: `Hot pastry → 2` (fire icon), `Cold pastry → 1`.
4. If `cost > budget`, stop. Otherwise append to `order.pastries[]` and subtract cost.

The budget approach means the order is built to fit the bubble — no trimming after the fact. `MAX_ORDER_ICONS = 5` is the single source of truth shared by `Order.h`, `Menu.cpp`, and `CustomerFactory.cpp`.

### Recipes (`MENU[]` in `src/Menu.h`)

| Drink | Coffee | Water | Milk | Temp | `iconFrame` | `targetFill` |
|-------|--------|-------|------|------|-------------|--------------|
| Black coffee | 0.80 | 0.20 | 0.00 | Hot or Cold | 5 | 1.0 |
| Espresso     | 1.00 | 0.00 | 0.00 | Hot only     | 6 | 0.5 |
| Americano    | 0.35 | 0.50 | 0.15 | Hot or Cold  | 7 | 1.0 |
| Latte        | 0.20 | 0.70 | 0.10 | Hot only     | 8 | 1.0 |
| Cappuccino   | 0.30 | 0.70 | 0.00 | Hot or Cold  | 9 | 0.75 |

`iconFrame` is an absolute index into `props.png`. `pastryFrom` is the first frame of the `"pastry"` tag in `props.json`; pastry icon = `pastryFrom + PastryType` (0-based).

Temperature frames: Hot = 10 (fire), Cold = 11 (ice) — `temperatureFrame()` in `Menu.h`.

---

## 2. Entity hierarchy — customer → bubble → icons

`spawnCustomer()` (`src/entity/CustomerFactory.cpp`) builds a three-level entity tree:

```
Customer entity
  └─ Speech bubble entity  (ChildOf: customer, offset {0, +28 px})
       ├─ Icon entity 0    (ChildOf: bubble, offset computed below)
       ├─ Icon entity 1
       └─ … up to 5 icons
```

**Customer entity** components:
- `Transform` — 32×48 px (from `PERSON_DIMS`), physics-body-synced position
- `Drawable` — random frame from `customers.png`/`customers.json`, layer `CUSTOMER`
- `Order` — filled by `randomOrder()` at spawn time
- `Behavior` — `patience` and `maxPatience` from the `Spawner`, `rating`/`succeeded` written at departure
- `PhysicsBody` — static body (sensor) sized to the Transform, used by `deliverySystem`
- `DropSpace{ DropType::Any }` — signals this entity accepts dropped items
- `OrderGrade` — per-slot scores and `servedMask` bitmask

**Speech bubble entity** (`src/entity/SpeechBubbleFactory.cpp`):
- `Transform` — 64×24 px (`BUBBLE_DIMS`), no physics
- `Drawable` — `bubble.png`, layer `UI1`
- `ChildOf{ customer, {0, 28} }` — 28 px below customer center in screen space; `TransformSystem::hierarchySystem` writes the world position each frame

**Order icon entities** (`src/entity/OrderIconFactory.cpp`):
- `Transform` — `ICON_SIZE × ICON_SIZE` in world units
- `Drawable` — single frame from `props.png`, layer `UI2`
- `ChildOf{ bubble, {x, 0} }` — offset x computed by layout math below

---

## 3. Icon layout math (`spawnCustomer`, `CustomerFactory.cpp`)

```
BUBBLE_W  = 64          // px
SLOT_W    = 64 / 5      // = 12.8 px per slot
ICON_SIZE = 12.8 * 0.85 // ≈ 10.9 px  (fits slot with small gap)

slot i center x = -32 + (i + 0.5) * 12.8
```

Slot positions relative to bubble center:

| i | x (px) |
|---|--------|
| 0 | −25.6 |
| 1 | −12.8 |
| 2 |   0.0 |
| 3 | +12.8 |
| 4 | +25.6 |

Icons are pushed left-to-right in `frames[]`. The push order mirrors the budget loop: all drink icons first (plus ice icon immediately after each cold drink), then all pastry icons (plus fire icon immediately after each hot pastry). The result is a left-aligned compact strip — empty slots stay invisible because no entity is created for them.

---

## 4. Frame index resolution

```
coffeeFrom = props.getTagBounds("coffee").first  // first frame of "coffee" tag
pastryFrom = props.getTagBounds("pastry").first  // first frame of "pastry" tag

drink icon   = coffeeFrom + static_cast<int>(order.drinks[i].type)
pastry icon  = pastryFrom + static_cast<int>(order.pastries[i].type)
temp icon    = temperatureFrame(temp)  // 10=fire  11=ice
```

`validateOrderSprites()` called at startup checks that `PastryType::count` and `ItemTypes::count` never exceed the corresponding tag's frame count in `props.json`.

---

## 5. Spawner → customer flow

```
Spawner entity   (one per seat, never destroyed)
   Spawner{seat, patience, interval, cooldown}

customerSpawnerSystem (each frame):
  if any customer exists → reset cooldown, skip
  cooldown -= dt
  if cooldown <= 0:
      spawnCustomer(assets, physics, seat, randomOrder(), patience)
      cooldown = interval
```

Only one customer can exist at a time (seat-level check). When the customer departs (`customerCleanupSystem` destroys it), the seat becomes empty and the interval timer starts.

---

## 6. Order ↔ customer relationship

The `Order` struct is stored directly as a component on the customer entity — no separate entity, no indirection. Systems that need the order read `customer.get<Order>()`.

`OrderGrade` is also on the customer entity and tracks which slots have been served via a bitmask (`servedMask`). The grading pipeline (see `docs/grading-system.md`) writes scores into `OrderGrade` when items are delivered.

`Leaving{}` tag is added when:
- `orderSystem` detects `allItemsServed()` returns true, or
- `behaviorSystem` finds `patience <= 0`

`finalizeOrderGradeSystem` runs for entities with `Leaving + Behavior + Order + OrderGrade`, computes the penalized rating, and writes `Behavior.rating` and `Behavior.succeeded`.

---

## 7. Full spawn-to-departure flow

```
Spawner cooldown expires
    ↓
randomOrder()           // Menu.cpp — budget-capped random order
    ↓
spawnCustomer()         // CustomerFactory.cpp
    ├─ createCustomer() → customer entity (Transform, Drawable, Order, Behavior,
    │                      PhysicsBody, DropSpace, OrderGrade)
    ├─ createSpeechBubble() → bubble entity (ChildOf customer +28 px)
    └─ createOrderIcon() × n → icon entities (ChildOf bubble, left-aligned row)
    ↓
Each frame:
    behaviorSystem()           — patience ticks down; patience≤0 → add Leaving
    hierarchySystem()          — bubble + icons follow customer position
    deliverySystem()           — player drops item → CheckCoffeeIntent / CheckPastryIntent
    checkBeverageSystem()      — builds CoffeeOverview from liquid drops in cup
    acceptGradedBeverageSystem() — grades drink 0-100, destroys cup
    orderSystem()              — allItemsServed? → add Leaving
    ↓
finalizeOrderGradeSystem()     — sum grades + patience penalty → Behavior.rating
reportLeavingCustomers()       — logs result
customerCleanupSystem()        — destroys customer entity (bubble + icons destroyed via ChildOf)
    ↓
Spawner sees 0 customers → cooldown starts again
```

---

## Key files

| File | Role |
|------|------|
| `src/Menu.h` / `src/Menu.cpp` | `MENU[]` recipes, `randomOrder()`, `temperatureFrame()` |
| `src/component/Order.h` | `Order` struct, `MAX_ORDER_ICONS`, `MAX_DRINKS`, `MAX_PASTRIES` |
| `src/component/Behavior.h` | `Behavior` struct (patience, rating) |
| `src/component/Spawner.h` | `Spawner` struct (seat, interval, patience) |
| `src/entity/CustomerFactory.cpp` | `spawnCustomer()` — creates customer + bubble + icons |
| `src/entity/SpeechBubbleFactory.cpp` | `createSpeechBubble()` |
| `src/entity/OrderIconFactory.cpp` | `createOrderIcon()` |
| `src/system/CustomerSystem.cpp` | `customerSpawnerSystem`, `behaviorSystem`, `deliverySystem`, `orderSystem`, `finalizeOrderGradeSystem`, cleanup |
| `src/SpriteDims.h` | `BUBBLE_DIMS = {64, 24}`, `PERSON_DIMS = {32, 48}` |
| `src/ItemTypes.h` | `ItemTypes`, `PastryType`, `Temperature` enums |
