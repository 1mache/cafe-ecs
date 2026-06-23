/**
 * @file Text.h
 * @brief Bitmap-font text rendering.
 */
#pragma once

#include <SDL3/SDL.h>
#include <string_view>

#include "Texture.h"

namespace cafe
{
/** @brief Blit @p text as a scaled bitmap from @p font to @p renderer at (@p x, @p y).
 *
 *  Glyphs are looked up via Glyph::glyphIndex(char); unknown chars are skipped.
 *  Each glyph is GLYPH_W × GLYPH_H pixels; glyphs are tracked with GLYPH_GAP pixels.
 *  The @p scale is applied uniformly to all dimensions.
 *
 *  @param renderer  SDL3 renderer (non-null)
 *  @param font      Bitmap-font texture (5×7 glyphs, one per FONT_GLYPHS character, left-to-right)
 *  @param text      Text to render
 *  @param x         Left-top X coordinate in pixels
 *  @param y         Left-top Y coordinate in pixels
 *  @param scale     Integer scale factor (default 1)
 */
void drawText(SDL_Renderer* renderer, const Texture& font, std::string_view text,
              float x, float y, int scale = 1);

} // namespace cafe
