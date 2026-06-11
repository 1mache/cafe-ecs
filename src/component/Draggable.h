#pragma once

#include "DropType.h"
#include <bagel.h>
#include <box2d/box2d.h>

namespace cafe
{
/** @brief Entity can be picked up; dropType declares which DropSpace types it is compatible with. */
struct Draggable
{
    DropType dropType{ DropType::Any };
};

/** @brief Adds a DRAGGABLE sensor shape to body for drag hit-testing via Box2D sensor events. */
void addDraggableVisitorShape(b2BodyId body, float halfW, float halfH);

/** @brief Enables sensor events on all entities currently carrying a Held component. */
void enableSensorEventsOnHeldEntities();

} // namespace cafe

template <> struct bagel::Storage<cafe::Draggable> final : bagel::NoInstance { using type = bagel::SparseStorage<cafe::Draggable>; };
