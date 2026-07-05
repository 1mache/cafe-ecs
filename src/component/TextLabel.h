#pragma once

#include "Glyph.h" // TextAlign

#include <SDL3/SDL.h>
#include <bagel.h>
#include <algorithm> // std::min
#include <cstddef>   // std::size_t
#include <cstring>   // std::memcpy
#include <string_view>

namespace cafe
{
/** @brief Text drawn at the entity's own Transform position (world units,
 *  center x/y — w/h/rot unused, same convention as Transform-only hitbox
 *  entities like a MenuButton hitbox). Drawn by drawTextSystem.
 *
 *  Text lives in a fixed inline buffer (no heap): BufSize includes the
 *  terminating '\0', so at most MaxLen visible chars — longer text is
 *  silently truncated. Buffer is always null-terminated, so `text` decays to
 *  a valid C-string / std::string_view for the render path. */
struct TextLabel
{
    static constexpr std::size_t BufSize = 32;          // includes '\0'
    static constexpr std::size_t MaxLen  = BufSize - 1; // visible chars

    char        text[BufSize]{};
    float       scale{1.f};
    TextAlign   align{TextAlign::Left};
    SDL_Color   tint{255, 255, 255, 255};

    TextLabel() = default;
    TextLabel(std::string_view s, float scale_ = 1.f, TextAlign align_ = TextAlign::Left,
              SDL_Color tint_ = { 255, 255, 255, 255 })
        : scale{ scale_ }, align{ align_ }, tint{ tint_ }
    {
        setText(s);
    }

    void setText(std::string_view s)
    {
        const std::size_t n = std::min(s.size(), MaxLen);
        if (n)
            std::memcpy(text, s.data(), n);
        text[n] = '\0';
    }
};
} // namespace cafe

template <> struct bagel::Storage<cafe::TextLabel> final : bagel::NoInstance
{
    using type = bagel::SparseStorage<cafe::TextLabel>;
};
