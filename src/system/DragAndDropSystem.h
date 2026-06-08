#pragma once

#include <SDL3/SDL.h>

namespace cafe
{
/**
 * @brief Called on SDL_EVENT_MOUSE_BUTTON_DOWN.
 *        Hit-tests DropSpace entities at screenPos and re-enables sensor events on any hit body.
 */
void dropSpacePickupSystem(SDL_FPoint screenPos);

/**
 * @brief Called on SDL_EVENT_MOUSE_BUTTON_DOWN.
 *        Finds the topmost Draggable entity under screenPos and attaches a Held component.
 */
void dragStartSystem(SDL_FPoint screenPos);

/**
 * @brief Called every frame while a button is held.
 *        Moves all Held entities to follow the mouse (only when not hovering a DropSpace).
 */
void midDragSystem(SDL_FPoint screenPos);

/**
 * @brief Called after PhysicsContext::step() each frame.
 *        Reads Box2D sensor begin/end events and updates Held.dropSpaceEntity accordingly.
 */
void dropSpaceDetectionSystem();

/**
 * @brief Called on SDL_EVENT_MOUSE_BUTTON_UP.
 *        Snaps valid drops, restores/cancels gravity, disables sensor events, removes Held.
 */
void dragReleaseSystem();
} // namespace cafe
