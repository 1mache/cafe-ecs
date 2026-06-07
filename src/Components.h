#pragma once

#include "SDL3/SDL_render.h"
#include "WorldPos.h"

#include <bagel.h>
#include <box2d/id.h>

namespace cafe
{
// Definition of all components and their storage

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

struct ChildOf
{
    bagel::Entity parent{bagel::ent_type(-1)};
    SDL_FPoint localOffset{}; // from center of parenting object
};

/** @brief Tag — marks an entity as a coffee drop. */
struct Liquid {};

/** @brief Handle to a Box2D body. Always destroy via destroyPhysicalEntity. */
struct PhysicsBody
{
    b2BodyId id{ b2_nullBodyId };
};

/** @brief A cup that catches coffee drops. */
struct Cup
{
    int   capacity{};
    int   filled{};
    float fillPercent() const
    {
        return capacity ? static_cast<float>(filled) / static_cast<float>(capacity) : 0.f;
    }
    bool  isFull() const { return filled >= capacity; }
};

/** @brief Emits a stream of coffee drops while active. */
struct CoffeeSpawner
{
    float    interval{ 0.05f };  // seconds between drops
    float    accumulator{};
    bool     active{};
    WorldPos offset{};           // spawn point relative to the spawner's Transform
};

/** @brief Tag — marks the out-of-bounds cleanup sensor. */
struct CleanupZone {};

} // namespace cafe

template <> struct bagel::Storage<cafe::Transform>     final : NoInstance { using type = SparseStorage<cafe::Transform>; };
template <> struct bagel::Storage<cafe::Drawable>      final : NoInstance { using type = SparseStorage<cafe::Drawable>; };
template <> struct bagel::Storage<cafe::Liquid>        final : NoInstance { using type = SparseStorage<cafe::Liquid>; };
template <> struct bagel::Storage<cafe::PhysicsBody>   final : NoInstance { using type = SparseStorage<cafe::PhysicsBody>; };
template <> struct bagel::Storage<cafe::Cup>           final : NoInstance { using type = SparseStorage<cafe::Cup>; };
template <> struct bagel::Storage<cafe::CoffeeSpawner> final : NoInstance { using type = SparseStorage<cafe::CoffeeSpawner>; };
template <> struct bagel::Storage<cafe::CleanupZone>   final : NoInstance { using type = SparseStorage<cafe::CleanupZone>; };