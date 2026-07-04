#include <catch2/catch_test_macros.hpp>
#include "OrderGrade.h"

using namespace cafe;

TEST_CASE("gradeTier maps rating to a tier, with timeout overriding score")
{
    // 2 items => max possible = 2 * MAX_ITEM_GRADE (200).
    // succeeded=false always => Failed, regardless of a high rating.
    REQUIRE(gradeTier(200, 2, false) == GradeTier::Failed);

    // >= 90% of max => Perfect (180/200 = 0.9).
    REQUIRE(gradeTier(180, 2, true) == GradeTier::Perfect);

    // >= 60% and < 90% => Good (120/200 = 0.6).
    REQUIRE(gradeTier(120, 2, true) == GradeTier::Good);

    // < 60% => Meh (100/200 = 0.5).
    REQUIRE(gradeTier(100, 2, true) == GradeTier::Meh);

    // Zero items (defensive): no divide-by-zero, collapses to Meh.
    REQUIRE(gradeTier(0, 0, true) == GradeTier::Meh);
}
