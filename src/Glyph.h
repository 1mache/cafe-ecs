#pragma once

#include <string_view>

namespace cafe
{
/** @brief Glyph order in res/font.png — a single horizontal strip, one GLYPH_W x
 *  GLYPH_H cell per character, left to right, in exactly this order. */
inline constexpr std::string_view FONT_GLYPHS =
    " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ/-%";

inline constexpr int GLYPH_W   = 5; // px per glyph cell in font.png
inline constexpr int GLYPH_H   = 7; // px per glyph cell in font.png
inline constexpr int GLYPH_GAP = 1; // px tracking inserted between glyphs when drawing

/** @brief Column of @p c in FONT_GLYPHS (letters folded to uppercase), or -1. */
constexpr int glyphIndex(char c)
{
    if (c >= 'a' && c <= 'z')
        c = static_cast<char>(c - 'a' + 'A');
    for (size_t i = 0; i < FONT_GLYPHS.size(); ++i)
        if (FONT_GLYPHS[i] == c)
            return static_cast<int>(i);
    return -1;
}

/** @brief Rendered pixel width of @p text at integer @p scale. */
constexpr float textWidth(std::string_view text, int scale)
{
    const int n = static_cast<int>(text.size());
    if (n == 0) return 0.f;
    return static_cast<float>((n * GLYPH_W + (n - 1) * GLYPH_GAP) * scale);
}

/** @brief Horizontal anchor rule for @ref alignedX. */
enum class TextAlign { Left, Center, Right };

/** @brief Left-edge x for @p text at @p scale, positioned so it reads as
 *  @p align relative to @p anchorX (Left: starts at anchorX; Center: centered
 *  on anchorX; Right: ends at anchorX). */
constexpr float alignedX(std::string_view text, int scale, TextAlign align, float anchorX)
{
    switch (align)
    {
        case TextAlign::Center: return anchorX - textWidth(text, scale) * 0.5f;
        case TextAlign::Right:  return anchorX - textWidth(text, scale);
        default:                return anchorX; // Left
    }
}
} // namespace cafe
