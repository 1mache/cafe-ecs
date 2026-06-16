#include "OrderMatch.h"
#include <cmath>
#include <iostream>

namespace cafe
{
// for now only first drink
// TODO: add more drinks
DrinkGrade gradeDrink(const CheckCoffeeIntent& intent, const CoffeeOverview& overview)
{
    float grade = BASE_GRADE;

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

    if (BASE_GRADE - grade >= RATIO_TOL_PERFECT) return DrinkGrade::Perfect;
    if (BASE_GRADE - grade >= RATIO_TOL_ACCEPTABLE) return DrinkGrade::Acceptable;
    return DrinkGrade::Wrong;
}
} // namespace cafe
