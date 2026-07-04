#pragma once

#include <SDL3/SDL.h>

namespace cafe
{
class AudioContext;

/** @brief Single SDL poll for the shop screen (parallels intentSystem). Converts
 *  the mouse to world space and hit-tests every ShopButton / NextDayButton
 *  Transform. Sets ShopButton.justPressed on a hit; sets outNextDay when the
 *  next-day button (or Enter/Space) is pressed; sets outExit on window close.
 *  Plays a click on any button hit. */
void shopInputSystem(SDL_Renderer* renderer, bool& outNextDay, bool& outExit, AudioContext& audio);

/** @brief Consume pressed ShopButtons: buy the upgrade (tryBuy no-ops if
 *  unaffordable or maxed) and clear the flag. Plays a sound only on a real purchase. */
void shopPurchaseSystem(AudioContext& audio);
} // namespace cafe
