#pragma once

#include <SDL3/SDL.h>

namespace cafe
{
class AudioContext;

/** @brief Single SDL poll for the shop screen (parallels menuInputSystem).
 *  Converts a mouse-down to world space and calls updateButtonsFromMouse over
 *  every Button entity. Sets outNextDay on Enter/Space (the Shop/Menu Button
 *  hits themselves are read by DayReportScene afterward via Button.pressed).
 *  Sets outExit on window close. */
void shopInputSystem(SDL_Renderer* renderer, bool& outNextDay, bool& outExit, AudioContext& audio);
} // namespace cafe
