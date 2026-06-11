# Roadmap

## Required code changes

- **InputSystem** — replace direct SDL event polling with an `InputSystem` that exposes state via a component. Ref: code-conventions.md.
- **Remove globals** — eliminate the global coffee machine reference and the global `isDragging` state. Encapsulate `PhysicsContext` and `RenderContext` inside the appropriate `Scene`.
- **Liquid fill** — liquid particles should fill the bottom half of the cup so the cup can be poured out (liquid transfers out when tipped).

---

## Features

### Coffee machine
- Fully working machine dispensing different liquid types from different pipes/tubes.
- Restocking: refill coffee beans and milk. Water is always available (piped in).

### Ice
- Ice container. Player grabs a cube and drops it into a cup for iced drinks.

### Order & queue system
- Proper customer queue with entry and exit animations.
- Further development of the ordering pipeline.

### Recipe ratios & order verification
- Display the requested ratio (Milk / Water / Coffee) visually in the customer's speech bubble.
- Verification allows reasonable tolerance — e.g. 43/57 counts as 50/50.
- Define standard real-life recipes; customer orders one at random. A recipe cheat-sheet is visible in-game.

**Example — Americano:**
| Ingredient | Ratio |
|---|---|
| Water | 40% |
| Milk | 10% |
| Coffee | 50% |

### Pastries
- Currently a basic container that instantiates them. Restocking logic TBD — may simply reappear on request for now.

### Audio
- Sound effects and background music.

### Main menu
- Primary landing/title screen.
