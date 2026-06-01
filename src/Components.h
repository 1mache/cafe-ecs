#pragma once

#include "SDL3/SDL_render.h"

#include <bagel.h>

namespace cafe
{
// Definition of all components and their storage

inline constexpr int INGREDIENT_KINDS = 3;

/** @brief Indices into Order::ratio — [Coffee=0, Milk=1, Tea=2]. */
enum class Ingredient { Coffee, Milk, Tea };

/** @brief World-space AABB transform: center (x,y), half-extents (w,h), rotation. */
struct Transform
{
    // *In world coordinates
    float x{}, y{}; // center
    float w{}, h{}; // from the center. like box2d
    // degrees or radians but can be converted into another type later
    float rot{};
};

struct Drawable
{
    SDL_Texture* texture{};
    SDL_FRect    srcRect{};
    int          renderLayer{};
};

/** @brief A client's order: a desired drink composition and/or a pastry.
 *  Invariant: hasDrink || hasPastry must be true. */
struct Order
{
    int  ratio[INGREDIENT_KINDS]{}; // desired drink mix: [coffee, milk, tea]
    bool hasDrink{};
    bool hasPastry{};               // single pastry type for now
};

/** @brief A client's runtime patience and satisfaction rating. */
struct Behavior
{
    float patience{}; // seconds remaining before the client leaves
    int   rating{};   // satisfaction score written after order is served
};

} // namespace cafe

template <>
struct bagel::Storage<cafe::Transform> final : NoInstance
{
    using type = SparseStorage<cafe::Transform>;
};

template <>
struct bagel::Storage<cafe::Drawable> final : NoInstance
{
    using type = SparseStorage<cafe::Drawable>;
};

template <>
struct bagel::Storage<cafe::Order> final : NoInstance
{
    using type = SparseStorage<cafe::Order>;
};

template <>
struct bagel::Storage<cafe::Behavior> final : NoInstance
{
    using type = SparseStorage<cafe::Behavior>;
};