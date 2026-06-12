#pragma once

#include <bagel.h>
#include <SDL3/SDL.h>
#include <optional>
#include "DropType.h"
enum class DragIntentType
{
    None,
    held,
    released
};

namespace cafe
{
/** @brief Added to an entity while it is actively being dragged by the player. */
struct DragIntent
{
    DragIntentType                 intentType{ DragIntentType::None };
    SDL_FPoint                     mousePos{};
    DropType                       dropType{ DropType::Any };//what entity is being dragged
    std::optional<bagel::ent_type> dropSpaceEntity{ std::nullopt };
};
} // namespace cafe

template <> struct bagel::Storage<cafe::DragIntent> final : bagel::NoInstance { using type = bagel::SparseStorage<cafe::DragIntent>; };
