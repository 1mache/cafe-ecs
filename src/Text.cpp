/**
 * @file Text.cpp
 * @brief Bitmap-font text rendering.
 */

#include "Text.h"

#include "Glyph.h"

namespace cafe
{
void drawText(SDL_Renderer* renderer, const Texture& font, std::string_view text,
              float x, float y, float scale)
{
    float xpos = x;

    for (char c : text)
    {
        const int idx = glyphIndex(c);
        if (idx < 0)
            continue; // unknown glyph, skip

        // Glyph is at column idx in the font strip
        const float src_x = static_cast<float>(idx * GLYPH_W);
        const float src_y = 0.f;
        const SDL_FRect src_rect{src_x, src_y, static_cast<float>(GLYPH_W),
                                 static_cast<float>(GLYPH_H)};

        // Destination: scaled glyph at (xpos, y)
        const float dst_w = static_cast<float>(GLYPH_W) * scale;
        const float dst_h = static_cast<float>(GLYPH_H) * scale;
        const SDL_FRect dst_rect{xpos, y, dst_w, dst_h};

        SDL_RenderTexture(renderer, font.get(), &src_rect, &dst_rect);

        // Advance x by glyph width + tracking, scaled
        xpos += static_cast<float>(GLYPH_W + GLYPH_GAP) * scale;
    }
}
} // namespace cafe
