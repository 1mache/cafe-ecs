#pragma once

namespace cafe
{
/** @brief Every upgrade the shop sells. `count` is last so it equals the number
 *  of real upgrades (sizes UPGRADES[] and bounds loops). Add an upgrade by adding
 *  an enumerator here (before count) and a matching row in UPGRADES[]. */
enum class UpgradeId { TapSpeed, MicrowaveSpeed, KeyboardPour, KeyboardPastry, count };

/** @brief Data-driven definition of one upgrade. Cost and effect are linear in
 *  the current level: cost(lvl) = baseCost + lvl*costStep,
 *  value(lvl) = baseValue + lvl*valueStep. */
struct UpgradeDef
{
    const char* name;      // shown in the shop
    int   maxLevel;        // level cap (change per-upgrade here)
    int   baseCost;        // price to buy level 1 (from level 0)
    int   costStep;        // added to cost for each further level
    float baseValue;       // effective value at level 0
    float valueStep;       // change to value per level (negative = "faster")
};

// THE table the team edits. Index order MUST match UpgradeId.
inline constexpr UpgradeDef UPGRADES[static_cast<int>(UpgradeId::count)] = {
    //  name                maxLvl baseCost costStep baseValue valueStep
    { "FASTER TAPS",        3,     100,      50,      0.05f,    -0.011f },
    { "FASTER MICROWAVE",   3,     100,      50,      5.5f,     -1.f   },
    { "KEYBOARD POUR",      1,     200,      0,       1.f,      0.f     },
    { "KEYBOARD PASTRY",    1,     200,      0,       1.f,      0.f     },
};

const UpgradeDef& upgradeDef(UpgradeId id);
/** @brief Cost to buy the NEXT level given the current level. */
int   upgradeCost(UpgradeId id, int currentLevel);
/** @brief Effective value at the given level. */
float upgradeValue(UpgradeId id, int level);
/** @brief True when level is at or beyond this upgrade's cap. */
bool  isMaxed(UpgradeId id, int level);
} // namespace cafe
