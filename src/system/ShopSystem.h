#pragma once

#include <SDL3/SDL.h>

namespace cafe
{
/** @brief Single SDL poll for the shop screen (parallels intentSystem). Converts
 *  the mouse to world space and hit-tests every ShopButton / MenuButton
 *  Transform. Sets ShopButton.justPressed on a hit; sets outNextDay when a
 *  MenuAction::NextDay button (or Enter/Space) is pressed; sets outExit on
 *  window close. */
void shopInputSystem(SDL_Renderer* renderer, bool& outNextDay, bool& outExit);

/** @brief Consume pressed ShopButtons: buy the upgrade (tryBuy no-ops if
 *  unaffordable or maxed) and clear the flag. */
void shopPurchaseSystem();
} // namespace cafe
