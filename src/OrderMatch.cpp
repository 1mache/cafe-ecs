#include "OrderMatch.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace cafe
{
int gradeDrink(const CheckCoffeeIntent& intent, const CoffeeOverview& overview)
{
    if (overview.dropSum == 0) return 0;

    float grade = 1.0f;

    for (size_t i = 0; i < INGREDIENT_COUNT; ++i)
    {
        const float want = intent.ratio[i];
        const float got  = overview.ratio[i];
        grade -= std::fabs(want - got);
        std::cout << "ingredient: " << i << " expected: " << want << " got: " << got
                  << " grade: " << grade << std::endl;
    }

    const float dropSumDiff = static_cast<float>(overview.dropSum - intent.dropSum);
    grade -= std::fabs(dropSumDiff) / 100.0f;

    std::cout << "dropSumDiff: " << dropSumDiff << " grade: " << grade << std::endl;

    const float clamped = std::clamp(grade, 0.0f, 1.0f);
    return static_cast<int>(std::round(clamped * 100.0f));
}
} // namespace cafe
