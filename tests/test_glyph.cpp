#include <catch2/catch_test_macros.hpp>
#include "Glyph.h"

using namespace cafe;

TEST_CASE("glyphIndex maps characters to font-strip columns")
{
    REQUIRE(glyphIndex(' ') == 0);
    REQUIRE(glyphIndex('0') == 1);
    REQUIRE(glyphIndex('7') == 8);
    REQUIRE(glyphIndex('A') == 11);
    REQUIRE(glyphIndex('a') == 11);   // lowercase folds to uppercase
    REQUIRE(glyphIndex('Z') == 36);
    REQUIRE(glyphIndex('/') == 37);
    REQUIRE(glyphIndex('-') == 38);
    REQUIRE(glyphIndex('?') == -1);   // unknown glyph
}

TEST_CASE("textWidth accounts for glyph width, tracking, and scale")
{
    REQUIRE(textWidth("", 1) == 0.f);
    REQUIRE(textWidth("AB", 1) == static_cast<float>(2 * GLYPH_W + 1 * GLYPH_GAP)); // 11
    REQUIRE(textWidth("AB", 2) == static_cast<float>((2 * GLYPH_W + 1 * GLYPH_GAP) * 2)); // 22
}
