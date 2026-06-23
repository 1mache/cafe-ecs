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
 *          - held:     follows the mouse.
 *          - released: snaps valid drops / restores gravity, resets intent to None.
 *        Sensors stay permanently enabled; drop-space gating is done in software
 *        via DragIntent state in dropSpaceDetectionSystem (no sensor toggling).
 */
void dragAndDropSystem();

/**
 * @brief Called after PhysicsContext::step() each frame.
 *        Reads Box2D sensor begin/end events and updates DragIntent.dropSpaceEntity.
 *        Drop-type matching uses DragItemType.dropType on the visitor.
 */
void dropSpaceDetectionSystem(PhysicsContext& physics);
} // namespace cafe
