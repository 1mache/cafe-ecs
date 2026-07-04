#pragma once

#include <bagel.h>

namespace cafe
{
/** @brief What a MenuButton click signals to its scene. */
enum class MenuAction
{
    Start,   // start-menu START square -> begin the game
    Exit,    // start-menu EXIT square  -> quit
    NextDay, // day-report power button -> advance to the next day
};

/** @brief A clickable Transform square that signals a scene transition.
 *  `action` selects which signal a hit raises (see menuInputSystem /
 *  shopInputSystem). Generalises the former StartButton / ExitButton /
 *  NextDayButton tags: same Transform hit-test, one Mask, no per-button type.
 *  A hit ends/advances the scene immediately via an out-param, so there is no
 *  justPressed consume step (unlike SoundToggleButton / ShopButton). */
struct MenuButton
{
    MenuAction action{ MenuAction::Start };
};
} // namespace cafe

template <> struct bagel::Storage<cafe::MenuButton> final : NoInstance
{ using type = SparseStorage<cafe::MenuButton>; };
