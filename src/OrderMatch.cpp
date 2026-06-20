#include "OrderMatch.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace cafe
{
int gradeDrink(const CheckCoffeeIntent& intent, const CoffeeOverview& overview)
{
    if (overview.dropSum == 0) return 0;

    float ratioGrade = 1.0f;
    for (size_t i = 0; i < INGREDIENT_COUNT; ++i)
    {
        const float want = intent.ratio[i];
        const float got  = overview.ratio[i];
        ratioGrade -= std::fabs(want - got);
        std::cout << "ingredient: " << i << " expected: " << want << " got: " << got
                  << " ratioGrade: " << ratioGrade << std::endl;
    }
    ratioGrade = std::clamp(ratioGrade, 0.0f, 1.0f);

    const float volumeGrade = std::clamp(
        1.0f - std::fabs(overview.fillPercent - intent.targetFill),
        0.0f, 1.0f);

    std::cout << "fillPercent: " << overview.fillPercent
              << " targetFill: " << intent.targetFill
              << " volumeGrade: " << volumeGrade << std::endl;

    const float grade = 0.8f * ratioGrade + 0.2f * volumeGrade;
    return static_cast<int>(std::round(std::clamp(grade, 0.0f, 1.0f) * 100.0f));
}
} // namespace cafe
