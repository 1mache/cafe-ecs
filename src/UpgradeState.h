#pragma once

#include "UpgradeCatalog.h"

namespace cafe
{
/** @brief Process-global economy: persistent money + per-upgrade levels. Survives
 *  scene teardown (the ECS world is destroyed between scenes), like DayState.
 *  Static service, no instances. */
class UpgradeState
{
public:
    static int money()             { return _money; }
    static int level(UpgradeId id) { return _level[static_cast<int>(id)]; }

    /** @brief Add a finished day's score to the persistent wallet. */
    static void bankScore(int score) { _money += score; }

    /** @brief Buy the next level of @p id. No-op returning false if the upgrade is
     *  maxed or unaffordable; otherwise deducts the cost, raises the level, returns true. */
    static bool tryBuy(UpgradeId id)
    {
        const int lvl = level(id);
        if (isMaxed(id, lvl)) return false;
        const int cost = upgradeCost(id, lvl);
        if (_money < cost) return false;
        _money -= cost;
        ++_level[static_cast<int>(id)];
        return true;
    }

    static void resetAll()
    {
        _money = 0;
        for (int& l : _level) l = 0;
    }

private:
    static inline int _money{ 0 };
    static inline int _level[static_cast<int>(UpgradeId::count)]{};
};
} // namespace cafe
