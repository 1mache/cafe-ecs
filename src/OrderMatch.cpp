#include "OrderMatch.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace cafe
{
namespace
{
static constexpr const char* kIngredientNames[] = { "Coffee", "Water", "Milk" };

static constexpr float kRatioGradeWeight          = 0.65f;
static constexpr float kVolumeGradeWeight         = 0.35f;
static constexpr float kNormalizedGradeMin        = 0.0f;
static constexpr float kNormalizedGradeMax        = 1.0f;
static constexpr int   kMaxDrinkScore             = 100;
static constexpr int   kColdMatchBonus            = 25;
static constexpr float kTempMismatchPenaltyFactor = 0.5f;

static float ratioGradeDivisor(int expectedTypeCount)
{
    switch (expectedTypeCount)
    {
    case 1: return 1.0f;
    case 2: return 2.0f;
    case 3: return 2.5f;
    default: return 1.0f;
    }
}
} // namespace

int gradeDrink(const DrinkRecipe& recipe, bool expectedHot, const CoffeeOverview& overview)
{
    if (overview.dropSum == 0)
    {
#ifdef DEBUG
        std::cout << "[BeverageScore] (empty cup) -> final score 0\n";
#endif
        return 0;
    }

    float totalDelta         = 0.0f;
    int   expectedTypeCount  = 0;
    for (size_t i = 0; i < INGREDIENT_COUNT; ++i)
    {
        const float want  = recipe.ratio[i];
        const float got   = overview.ratio[i];
        const float delta = std::fabs(want - got);
        totalDelta += delta;
        if (want > 0.0f)
        {
            ++expectedTypeCount;
        }
#ifdef DEBUG
        std::cout << "[BeverageScore] " << kIngredientNames[i]
                  << ": expected " << want << ", got " << got
                  << ", delta " << delta << "\n";
#endif
    }

    const float divisor             = ratioGradeDivisor(expectedTypeCount);
    const float ratioGradeUnclamped = 1.0f - totalDelta / divisor;
#ifdef DEBUG
    std::cout << "[BeverageScore] ratio: totalDelta " << totalDelta
              << ", expectedTypeCount " << expectedTypeCount
              << ", divisor " << divisor
              << ", ratioGrade " << ratioGradeUnclamped << "\n";
#endif

    const float ratioGradeBeforeClamp = ratioGradeUnclamped;
    const float ratioGrade            = std::clamp(ratioGradeUnclamped, kNormalizedGradeMin, kNormalizedGradeMax);
#ifdef DEBUG
    if (ratioGradeBeforeClamp != ratioGrade)
    {
        std::cout << "[BeverageScore] ratioGrade clamped: " << ratioGradeBeforeClamp
                  << " -> " << ratioGrade << "\n";
    }
#endif

    const float volumeGrade = std::clamp(
        1.0f - std::fabs(overview.fillPercent - recipe.targetFill),
        kNormalizedGradeMin, kNormalizedGradeMax);

#ifdef DEBUG
    std::cout << "[BeverageScore] fill: expected " << recipe.targetFill
              << " (targetFill), got " << overview.fillPercent
              << " (fillPercent), volumeGrade " << volumeGrade << "\n";
#endif

    const float grade        = kRatioGradeWeight * ratioGrade + kVolumeGradeWeight * volumeGrade;
    const float clampedGrade = std::clamp(grade, kNormalizedGradeMin, kNormalizedGradeMax);
    int         score        = static_cast<int>(std::round(clampedGrade * static_cast<float>(kMaxDrinkScore)));

#ifdef DEBUG
    std::cout << "[BeverageScore] weighted: " << kRatioGradeWeight << " * " << ratioGrade
              << " + " << kVolumeGradeWeight << " * " << volumeGrade << " = " << grade << "\n";
    std::cout << "[BeverageScore] base score: " << score << "\n";
    std::cout << "[BeverageScore] temperature: expected "
              << (expectedHot ? "Hot" : "Cold") << ", actual "
              << (overview.isHot ? "Hot" : "Cold") << " (ice present: "
              << (overview.isHot ? "no" : "yes") << ")\n";
#endif

    if (!expectedHot && !overview.isHot)
    {
        const int scoreBeforeBonus = score;
        score                      = std::min(kMaxDrinkScore, score + kColdMatchBonus);
#ifdef DEBUG
        std::cout << "[BeverageScore] cold match bonus: " << scoreBeforeBonus
                  << " + " << kColdMatchBonus << " -> " << score << "\n";
#endif
    }

    if (expectedHot != overview.isHot)
    {
        const int scoreBeforePenalty = score;
        score                        = static_cast<int>(std::round(static_cast<float>(score) * kTempMismatchPenaltyFactor));
#ifdef DEBUG
        std::cout << "[BeverageScore] temperature mismatch penalty: " << scoreBeforePenalty
                  << " * " << kTempMismatchPenaltyFactor << " -> " << score << "\n";
#endif
    }

#ifdef DEBUG
    std::cout << "[BeverageScore] final score: " << score << "\n";
#endif

    return score;
}
} // namespace cafe
