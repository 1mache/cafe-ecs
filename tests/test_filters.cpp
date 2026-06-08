#include "PhysicsFilters.h"
#include <catch2/catch_test_macros.hpp>

using namespace cafe::filter;

// LIQUID must never match a teammate-reserved bit. If their dropspace
// sensor uses (1ull << 4) and our mask widens by mistake, this would
// be the first thing to catch it.
TEST_CASE("MASK_LIQUID does not touch teammate-reserved bits 4-7", "[filters]")
{
    STATIC_REQUIRE((MASK_LIQUID & (1ull << 4)) == 0);
    STATIC_REQUIRE((MASK_LIQUID & (1ull << 5)) == 0);
    STATIC_REQUIRE((MASK_LIQUID & (1ull << 6)) == 0);
    STATIC_REQUIRE((MASK_LIQUID & (1ull << 7)) == 0);
}

TEST_CASE("Each feature category occupies exactly one bit", "[filters]")
{
    auto popcount = [](uint64_t x) {
        int c = 0;
        while (x) { x &= (x - 1); ++c; }
        return c;
    };
    REQUIRE(popcount(LIQUID)     == 1);
    REQUIRE(popcount(CUP_SOLID)  == 1);
    REQUIRE(popcount(CUP_INSIDE) == 1);
    REQUIRE(popcount(CLEANUP)    == 1);
}

TEST_CASE("Categories do not overlap each other", "[filters]")
{
    STATIC_REQUIRE((LIQUID      & CUP_SOLID)  == 0);
    STATIC_REQUIRE((LIQUID      & CUP_INSIDE) == 0);
    STATIC_REQUIRE((LIQUID      & CLEANUP)    == 0);
    STATIC_REQUIRE((CUP_SOLID   & CUP_INSIDE) == 0);
    STATIC_REQUIRE((CUP_SOLID   & CLEANUP)    == 0);
    STATIC_REQUIRE((CUP_INSIDE  & CLEANUP)    == 0);
}
