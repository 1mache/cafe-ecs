#pragma once

#include "AudioContext.h"
#include <SDL3/SDL.h>

namespace cafe
{
/** @brief Single SDL poll for the start menu (parallels shopInputSystem).
 *  Converts a mouse-down to world space and calls updateButtonsFromMouse over
 *  every Button entity. Sets outExit on window close. The Menu-button hit
 *  (Start/Exit) is read by the scene afterward via Button{kind==Menu}.pressed;
 *  this system no longer maps it itself. Mouse only — the menu has no key
 *  bindings. */
void menuInputSystem(SDL_Renderer* renderer, bool& outExit, AudioContext& audio);

/** @brief Re-tints every Button{kind==Sound} square from SettingsState::muted()
 *  each frame (white = sound on, gray = muted). The toggle action itself
 *  (flipping muted + applying volume) now lives in buttonDispatchSystem —
 *  this system is tint-only. */
void soundToggleSystem();
} // namespace cafe
