#pragma once

#include "Glyph.h" // TextAlign

#include <SDL3/SDL.h>
#include <bagel.h>
#include <string>

namespace cafe
{
/** @brief Text drawn at the entity's own Transform position (world units,
 *  center x/y — w/h/rot unused, same convention as Transform-only hitbox
 *  entities like NextDayButton). Drawn by drawTextSystem. */
struct TextLabel
{
    std::string text;
    int         scale{1};
    TextAlign   align{TextAlign::Left};
    SDL_Color   tint{255, 255, 255, 255};
};
} // namespace cafe

template <> struct bagel::Storage<cafe::TextLabel> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::TextLabel>;
};
