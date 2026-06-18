# Grading System

## Components involved

| Component | Owner | Purpose |
|-----------|-------|---------|
| `CheckCoffeeIntent` | Cup entity | Expected recipe + target customer; triggers grading pipeline |
| `CoffeeOverview` | Cup entity | Snapshot of actual cup contents (ratios, dropSum) |
| `Served` | Customer entity | Tracks whether drink/pastry delivered + final `DrinkGrade` |
| `DeliveredTo` | Cup entity | Marks cup as handed off; triggers cleanup |
| `Leaving` | Customer entity | Signals customer is done (success or patience ran out) |

---

## Pipeline

### 1. Cup dropped on customer — `deliverySystem`

Player releases cup via drag. `deliverySystem` sees `DragIntent` with `intentType == released` pointing at a customer. Builds `CheckCoffeeIntent` from the customer's order recipe and attaches it to the cup:

```cpp
CheckCoffeeIntent {
    ratio[INGREDIENT_COUNT]  // expected fractions from recipeFor(order.drinks[0].type)
    isHot                    // from order temperature
    customer                 // target customer entity id
}
```

Cup now has `Cup + CheckCoffeeIntent`. No grading yet.

---

### 2. `checkBeverageSystem` — build `CoffeeOverview`

Runs every frame. Finds entities with `Cup + CheckCoffeeIntent`. For each, `buildOverview(cupId)`:
- Iterates all `Liquid` entities in the world
- Keeps only drops whose `holdingContainer.id == cupId`
- Accumulates per-ingredient drop counts + total `dropSum`
- Normalizes: `ratio[i] = filled[i] / dropSum`
- Writes result into `CoffeeOverview` on the cup (adds or overwrites each frame)

---

### 3. `acceptGradedBeverageSystem` — grade + hand off

Finds entities with `Cup + CoffeeOverview + CheckCoffeeIntent`. Calls `gradeDrink(intent, overview)`:

```cpp
// OrderMatch.cpp
float grade = BASE_GRADE;  // 1.0
for each ingredient:
    grade -= fabs(intent.ratio[i] - overview.ratio[i]);
grade -= fabs(dropSum difference) / 100.0f;
```

Maps accumulated error (`BASE_GRADE - grade`) to `DrinkGrade`:

| Threshold | Grade |
|-----------|-------|
| error < `RATIO_TOL_PERFECT` (0.05) | `Perfect` |
| error < `RATIO_TOL_ACCEPTABLE` (0.12) | `Acceptable` |
| otherwise | `Wrong` |

> **Note:** `OrderMatch.cpp:27-28` comparison operators are currently inverted — conditions use `>=` where `<=` is needed. Drinks grade incorrectly until fixed.

After grading:
- Writes `drinkGrade` + `served.drink = true` into customer's `Served` component
- Removes `CheckCoffeeIntent` from cup
- Adds `DeliveredTo{customer}` to cup

---

### 4. `orderSystem` — complete or wait

Checks customers with `Order + Behavior + Served`. Once drink (and pastry if ordered) are both served:

| `DrinkGrade` | `Behavior.rating` |
|--------------|-------------------|
| `Perfect` | 2 |
| `Acceptable` | 1 |
| `Wrong` | 0 |

Adds `Leaving` tag to customer.

---

### 5. Cleanup

- `clearDeliveredItems` — destroys cups, liquid drops, and child sprites for leaving customers
- `customerCleanupSystem` — destroys the customer entity

---

## Flow summary

```
Cup dropped on customer
    → CheckCoffeeIntent added to cup
         ↓ (each frame)
    checkBeverageSystem
    → builds CoffeeOverview from Liquid drops in cup
         ↓
    acceptGradedBeverageSystem
    → gradeDrink(intent, overview) → DrinkGrade
    → writes Served.drinkGrade on customer
    → removes CheckCoffeeIntent, adds DeliveredTo
         ↓
    orderSystem
    → all items served? → set Behavior.rating → add Leaving
         ↓
    clearDeliveredItems + customerCleanupSystem
    → destroy cup, drops, customer
```

## Key files

| File | Role |
|------|------|
| `src/system/CheckBeverageSystem.cpp` | `checkBeverageSystem`, `acceptGradedBeverageSystem`, `buildOverview` |
| `src/OrderMatch.cpp` | `gradeDrink` — compares ratios, returns `DrinkGrade` |
| `src/OrderMatch.h` | `DrinkGrade` enum, tolerance constants |
| `src/component/CheckCoffeeIntent.h` | Expected recipe + customer ref |
| `src/component/CoffeeOverview.h` | Actual cup contents snapshot |
| `src/system/CustomerSystem.cpp` | `deliverySystem`, `orderSystem`, `gradeToRating`, cleanup |
