#pragma once

#include <box2d/box2d.h>

namespace cafe
{
class PhysicsContext;

/**
 * @brief Adds a DRAGGABLE sensor shape to body for drag hit-testing via Box2D sensor events.
 */
void addDraggableVisitorShape(b2BodyId body, float halfW, float halfH);

/**
 * @brief Single-pass drag pipeline; run before PhysicsContext::step().
 *        Switches on DragIntent.intentType per entity:
 *          - held:     enables sensor events and follows the mouse.
 *          - released: snaps valid drops / restores gravity, disables sensor
 *                      events, and resets the intent to None.
 *        Re-arms DropSpace sensors while anything is held.
 */
void dragAndDropSystem();

/**
 * @brief Called after PhysicsContext::step() each frame.
 *        Reads Box2D sensor begin/end events and updates DragIntent.dropSpaceEntity.
 *        Drop-type matching uses DragItemType.dropType on the visitor.
 */
void dropSpaceDetectionSystem(PhysicsContext& physics);
} // namespace cafe
