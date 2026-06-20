# Grading System

## Components involved

| Component | Owner | Purpose |
|-----------|-------|---------|
| `CheckCoffeeIntent` | Cup entity | Expected recipe + target customer + drink slot index; triggers grading pipeline |
| `CoffeeOverview` | Cup entity | Snapshot of actual cup contents (ratios, dropSum) |
| `OrderGrade` | Customer entity | Per-item numeric scores (0-100) + `servedMask` bitmask; computed immediately on delivery |
| `Leaving` | Customer entity | Signals customer is done (all items received, or patience ran out) |

---

## Item scores

### Drinks — continuous 0-100
`gradeDrink` in `OrderMatch.cpp` computes:

```
grade = 1.0
for each ingredient:
    grade -= fabs(expected_ratio - actual_ratio)
grade -= fabs(dropSum difference) / 100.0
score = round(clamp(grade, 0, 1) * 100)
```

### Pastries — flat or half
Full score (100) on delivery. Half score (50) if wrong temperature.

> **Stub:** pastry entities currently carry no temperature, so every delivered pastry earns 100.
> The half-points path is wired but inactive until pastries track temperature.

---

## Pipeline

### 1. Cup dropped on customer — `deliverySystem`

Player releases cup via drag. `deliverySystem` finds the first unserved drink slot on the customer's `OrderGrade`. If no slot is free, the cup bounces back to its slot. Otherwise it builds `CheckCoffeeIntent` from `order.drinks[slot]` and attaches it to the cup:

```cpp
CheckCoffeeIntent {
    ratio[INGREDIENT_COUNT]  // expected fractions from recipeFor(order.drinks[slot].type)
    isHot                    // from order temperature
    drinkSlot                // which drinks[] slot this cup fulfills
    customer                 // target customer entity id
}
```

Cup now has `Cup + CheckCoffeeIntent`. No grading yet.

---

### 2. Pastry dropped on customer — `deliverySystem`

`deliverySystem` finds the first unserved pastry slot on `OrderGrade`. If none is free, the pastry bounces back. Otherwise:
- Sets `pastryGrades[slot] = MAX_ITEM_GRADE` (100; stub: always full points).
- Calls `markPastryServed(grade, slot)`.
- Queues pastry for immediate destruction via `destroyDeliveredItem`.

---

### 3. `checkBeverageSystem` — build `CoffeeOverview`

Runs every frame. Finds entities with `Cup + CheckCoffeeIntent`. For each, `buildOverview(cupId)`:
- Iterates all `Liquid` entities in the world.
- Keeps only drops whose `holdingContainer.id == cupId`.
- Accumulates per-ingredient drop counts + total `dropSum`.
- Normalizes: `ratio[i] = filled[i] / dropSum`.
- Writes result into `CoffeeOverview` on the cup (adds or overwrites each frame).

---

### 4. `acceptGradedBeverageSystem` — grade + destroy cup

Finds entities with `Cup + CoffeeOverview + CheckCoffeeIntent`.

- Guard: if the customer no longer has `Order`/`OrderGrade` (left early), deletes `CheckCoffeeIntent` and destroys the cup.
- Calls `gradeDrink(intent, overview)` → `int` score 0-100.
- Writes `grade.drinkGrades[slot] = score` and calls `markDrinkServed(grade, slot)`.
- Removes `CheckCoffeeIntent` from cup.
- Destroys the cup entity immediately via `destroyDeliveredItem` (removes liquid drops, ice cubes, child sprites + the cup itself).

---

### 5. `orderSystem` — complete or wait

Checks customers with `Order + Behavior + OrderGrade`. Calls `allItemsServed(order, grade)`. If all drink and pastry slots are served and the customer is not already `Leaving`, adds `Leaving{}`.

Customers leave ONLY via `orderSystem` (full completion) or `behaviorSystem` (patience timeout).

---

### 6. `finalizeOrderGradeSystem` — compute penalized rating

Runs for every entity with `Leaving + Behavior + Order + OrderGrade`.

```
raw      = sumItemGrades(order, grade)   // sum of all per-item scores
factor   = patience <= 0        ? 0.75   // timed out: −25%
         : patience <= 50% max  ? 0.875  // half patience: −12.5%
         : 1.0                           // no penalty
rating   = round(raw * factor)
succeeded = allItemsServed(order, grade)
```

Writes `Behavior.rating` and `Behavior.succeeded`.

---

### 7. Cleanup

- `reportLeavingCustomers` — logs SUCCESSFUL/FAILED and final rating.
- `customerCleanupSystem` — destroys the customer entity.

---

## Flow summary

```
Cup dropped on customer
    → deliverySystem: find first unserved drink slot
    → CheckCoffeeIntent{slot} added to cup
         ↓ (each frame)
    checkBeverageSystem
    → builds CoffeeOverview from Liquid drops in cup
         ↓
    acceptGradedBeverageSystem
    → gradeDrink(intent, overview) → int 0-100
    → grade.drinkGrades[slot] set + markDrinkServed
    → cup + contents destroyed immediately (destroyDeliveredItem)
         ↓
Pastry dropped on customer
    → deliverySystem: find first unserved pastry slot
    → pastryGrades[slot] = 100, markPastryServed
    → pastry destroyed immediately (destroyDeliveredItem)
         ↓
    orderSystem
    → allItemsServed? → add Leaving
         ↓                                 (also: behaviorSystem → patience<=0 → add Leaving)
    finalizeOrderGradeSystem
    → sumItemGrades + patience penalty → Behavior.rating, Behavior.succeeded
         ↓
    reportLeavingCustomers + customerCleanupSystem
    → log result, destroy customer
```

## Key files

| File | Role |
|------|------|
| `src/system/CheckBeverageSystem.cpp` | `checkBeverageSystem`, `acceptGradedBeverageSystem`, `buildOverview` |
| `src/OrderMatch.cpp` | `gradeDrink` — compares ratios, returns int 0-100 |
| `src/OrderMatch.h` | `MIN_SERVE_FILL` constant |
| `src/component/CheckCoffeeIntent.h` | Expected recipe + customer ref + drink slot index |
| `src/component/CoffeeOverview.h` | Actual cup contents snapshot |
| `src/component/OrderGrade.h` | Per-item 0-100 scores + served bitmask + slot/sum helpers |
| `src/component/OrderGrade.cpp` | `firstUnservedDrink/Pastry`, `allItemsServed`, `sumItemGrades` |
| `src/entity/Entities.cpp` | `destroyDeliveredItem` — destroys a cup+contents or pastry immediately |
| `src/system/CustomerSystem.cpp` | `deliverySystem`, `orderSystem`, `finalizeOrderGradeSystem`, cleanup |
