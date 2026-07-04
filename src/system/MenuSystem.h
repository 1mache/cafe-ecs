#pragma once

#include "AudioContext.h"
#include <SDL3/SDL.h>

namespace cafe
{
/** @brief Single SDL poll for the start menu (parallels shopInputSystem).
 *  Converts the mouse to world space and hit-tests every MenuButton /
 *  SoundToggleButton Transform. A MenuButton hit sets outStart/outExit by its
 *  MenuAction; a sound-toggle hit sets SoundToggleButton.justPressed.
 *  Sets outExit on window close. Mouse only — the menu has no key bindings. */
void menuInputSystem(SDL_Renderer* renderer, bool& outStart, bool& outExit);

/** @brief Consume pressed SoundToggleButtons: flip SettingsState::muted and
 *  apply it to the audio device. Also re-tints every toggle square from the
 *  current state each frame (white = sound on, gray = muted), so the visual
 *  is always in sync — same state-driven-tint idiom as the shop bars. */
void soundToggleSystem(AudioContext& audio);
} // namespace cafe
