#include "UpgradeCatalog.h"

namespace cafe
{
const UpgradeDef& upgradeDef(UpgradeId id)
{
    return UPGRADES[static_cast<int>(id)];
}

int upgradeCost(UpgradeId id, int currentLevel)
{
    const UpgradeDef& d = upgradeDef(id);
    return d.baseCost + currentLevel * d.costStep;
}

float upgradeValue(UpgradeId id, int level)
{
    const UpgradeDef& d = upgradeDef(id);
    return d.baseValue + static_cast<float>(level) * d.valueStep;
}

bool isMaxed(UpgradeId id, int level)
{
    return level >= upgradeDef(id).maxLevel;
}
} // namespace cafe
