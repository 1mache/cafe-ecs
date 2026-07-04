#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief The menu's mute square. `justPressed` is set by menuInputSystem and
 *  consumed by soundToggleSystem (mirrors ShopButton): toggling mutates
 *  SettingsState and the square's tint, so it needs the press/consume split
 *  instead of ending the scene like StartButton/ExitButton. */
struct SoundToggleButton
{
    bool justPressed{};
};
} // namespace cafe

template <> struct bagel::Storage<cafe::SoundToggleButton> final : NoInstance
{ using type = SparseStorage<cafe::SoundToggleButton>; };
