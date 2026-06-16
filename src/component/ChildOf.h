#pragma once

#include <SDL3/SDL.h>
#include <bagel.h>

namespace cafe
{
struct ChildOf
{
    bagel::Entity parent{bagel::ent_type(-1)};
    SDL_FPoint localOffset{}; // from center of parenting object. px by default
    bool isWorldOffset{false};
};
} // namespace cafe

template <> struct bagel::Storage<cafe::ChildOf> final : NoInstance { using type = SparseStorage<cafe::ChildOf>; };
