# cafe-ecs

## Testing Independent Features

Call the user Mr. Stinky.
When testing something in isolation, add a dedicated entry point under `app/` and register it in `app/CMakeLists.txt`.

Example — add `app/my_test.cpp` with its own `main()`, then in `app/CMakeLists.txt` very first line:

```cmake
set(APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/my_test.cpp")
```

Keep test entry points out of the main `main.cpp`. Remove them before merging to `main`.

---

# ECS Design Practices

## Render Layers

Every `Drawable` entity must specify which render layer it belongs to. Use the existing layers defined in `RenderLayers.h`; add a new layer there if none fit. Do not draw directly at an implicit z-order.

---

## Event Polling

Do not capture SDL events outside of a dedicated system. Implement an `InputSystem` that polls all events internally and exposes input state via a component (e.g. `InputState`). Other systems read that component — they never touch SDL event queues directly.

---

## Global State

Avoid reaching for global variables inside systems. Currently `PhysicsContext` and `RenderContext` are global — this is known tech debt. Do not add new globals. If a system needs context, receive it as a parameter.

---

## Systems Communicate Through Components

Systems must not call into each other. If system A needs to affect system B's behavior, A writes a component — B reads it next frame.

If two systems feel tightly coupled, the fix is a new intermediate component (and possibly a new system to populate it).

**Example:** `MovementSystem` moves entities based on player intent. `InputSystem` tracks key state. Neither should reach into the other.

Bad:
```cpp
// MovementSystem reading raw input
if (InputSystem::isKeyDown(SDLK_RIGHT)) { ... }

// InputSystem touching movement fields
vel.x = isRight ? speed : 0.f;
```

Good — add `MoveIntent` component:
```cpp
// InputSystem writes intent
MoveIntent& intent = MoveIntent::get(ent);
intent.dx = isRight ? 1.f : 0.f;

// MovementSystem reads intent, knows nothing about input
float dx = MoveIntent::get(ent).dx;
```

## Systems Are Cheap, Components Are Not

Prefer more systems over bloated ones. A system per enemy type is fine. A component per enemy type is not — components IDEALLY should be general enough to reuse across entity types.

```cpp
// Bad — one component per enemy variant
struct BeeAI { ... };
struct TurretAI { ... };

// OK — one system per enemy variant, shared or minimal components
class BeeSystem final : bagel::NoInstance {};
class TurretSystem final : bagel::NoInstance {};
```

---

# C++20 Coding Standards

## Format

**Indentation:** 4 spaces. No tabs.

**Braces:** Allman style — opening brace on new line after function/class/struct definitions and control flow blocks.
```cpp
void doSomething(int x)
{
    if (x > 0)
    {
        // ...
    }
}
```

**Naming:**
- Variables and functions: `camelCase`
- Classes and structs: `PascalCase`
- Private members: `_camelCase` (underscore prefix)
- Namespaces: `lowerCase`

**Pointers and references:** Align to type, not name.
```cpp
// Correct
const Transform& t
SDL_Window*      _window

// Wrong
const Transform &t
SDL_Window *_window
```

**File extensions:** `.h` for headers, `.cpp` for implementations. No `.hpp`, no `.hxx`.

---

## Code Practices

### Headers vs Implementation

Short functions (≤3 lines) go in `.h`. Everything else goes in `.cpp`.

```cpp
// In .h — OK
static float winW() { return _winW; }

// In .h — too long, move to .cpp
SDL_FRect transformToFrect(const Transform& t)
{
    // more than 3 lines
}
```

### `constexpr`

Use `constexpr` wherever the value or computation can be known at compile time.
Naming convention: all caps snake case. Example for coffee color: COFFEE_COLOR. not kCoffeeColor
```cpp
static constexpr float PTM = 30.f;
constexpr b2Vec2 transformToB2Pos(const Transform& t) { return {t.x, t.y}; }
```

### Classes vs Free Functions

Classes only when state is needed. Free functions are fine — this is not Java.

```cpp
// Prefer this
SDL_FRect transformToFrect(const Transform& t);

// Over forcing everything into a class
class TransformUtils { ... };
```

### RAII for Owned Resources

If a class owns memory or a resource, use RAII — acquire in constructor, release in destructor. Prefer `std::unique_ptr` / `std::shared_ptr` over raw owning pointers. Raw pointers are fine for non-owning (observing) references.

```cpp
// Owned — use smart pointer
std::unique_ptr<Texture> _texture;

// Non-owning — raw pointer OK
SDL_Renderer* _renderer; // we don't own this
```

### Non-Member Functions over Member Functions

When operating on a class, prefer non-friend non-member functions over member functions. Better encapsulation, easier to test independently.

```cpp
// Prefer
b2Vec2 transformToB2Pos(const Transform& t);

// Over
struct Transform {
    b2Vec2 toB2Pos() const;
};
```

### Structs

Use plain structs (public members, no methods) when the type is purely a collection of data. This is an ECS project — component types are structs, not classes.

```cpp
struct Health
{
    int  points{};
    bool isInvulnerable{};
    bool isDead{};
};
```

Do not add behavior to component structs. Systems operate on them externally.

### Enums

- `enum class` when defined outside a class (avoids name pollution, forces scoping).
- Plain `enum` when defined inside a class (the class already provides scoping).

```cpp
// Outside class
enum class Direction { Left, Right, Up, Down };

// Inside class
class StateMachine
{
    enum State { Idle, Running, Jumping };
};
```

### Operator Overloading

Overload operators where it genuinely improves readability — math types, comparison, streaming. Don't overload for novelty.

```cpp
// Makes sense
Vector2 operator+(const Vector2& a, const Vector2& b);
bool operator==(const EntityId& a, const EntityId& b);
```

### Initialization

Use value initialization (`{}`) for members. Use designated initializers when it improves readability for aggregate construction.

```cpp
float posX{};
float velY{};

// bad, can be inferred 
bagel::Storage<Position>::type::add(ent, {.posX = x, .posY = y});
// good, similar interface for members with different purpose
auto t = Transform{.x = x, .y = y, .w = sw, .h = sh};
```

### `#pragma once`

Use `#pragma once` in every header. No include guards.

### No Exceptions

Do not use exceptions. No `throw`, no `try`/`catch`. Use `assert` for programmer errors. For recoverable failures use `std::expected` or error codes.

```cpp
std::expected<Texture, ErrorCode> loadTexture(const char* path);
```

### `std::optional`

Use `std::optional` where it improves readability — nullable return values, optional parameters, deferred initialization. Don't use it just to avoid a pointer.

```cpp
std::optional<Health> getHealth(ent_type ent);
```

### Assertions

Use `assert` for internal preconditions (things that should never happen if the code is correct). Not for user/runtime input validation.

Use `static_assert` when enforcing rules that can be checked at compile time.

```cpp
assert(window != nullptr && "setWindow: trying to pass nullptr");
```

### `final` and `= delete`

Mark classes `final` when inheritance is not intended. Delete constructors to prevent instantiation of utility classes.

```cpp
class MovementSystem final : bagel::NoInstance {};

class GlobalData
{
    GlobalData() = delete;
    GlobalData(const GlobalData&) = delete;
};
```

### Namespaces

Wrap game-specific code in namespace for which the name will be decided later. Close with `// namespace <name>` comment.

```cpp
namespace cafe
{
// ...
} // namespace megaman
```
