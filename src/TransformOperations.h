#pragma once
#include "WorldPos.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>

namespace cafe
{
struct Transform;

/**
 * @brief Converts a world point to SDL screen coordinates.
 *
 * Maps meters (Y-up, camera-relative) to pixels (Y-down, window-relative).
 * Formula: screenX = winCenterX + (worldX − camX) × PTM × scale
 *          screenY = winCenterY − (worldY − camY) × PTM × scale
 *
 * @param worldPos  Position in world units (meters, Y-up).
 * @param cam       Current camera center in world units.
 * @return SDL_FPoint in screen pixels (Y-down, origin top-left).
 */
SDL_FPoint worldToScreenPoint(WorldPos worldPos, WorldPos camPos);

/** @brief Converts a world-unit length to screen pixels (PTM × SCALE_FACTOR). */
constexpr float worldToScreenSize(float worldSize)
{
    return worldSize * PTM * SCALE_FACTOR;
}

/** @brief Converts a screen-pixel length to world units (inverse of worldToScreenSize). */
constexpr float screenToWorldSize(float screenSize)
{
    return screenSize / (PTM * SCALE_FACTOR);
}

/** @brief Converts an MTransform to an SDL_FRect in screen pixels for rendering. */
SDL_FRect transformToFrect(const Transform& t, WorldPos camPos);

b2Vec2 transformToB2Pos(const Transform& t);

b2Vec2 transformToB2Scale(const Transform& t);

void transformUpdateWithB2Pos(Transform& t, b2Vec2 pos);

}