#include "UpgradeSystem.h"

#include "Components.h"     // LiquidSpawner, Microwave
#include "UpgradeCatalog.h"
#include "UpgradeState.h"
#include <bagel.h>

namespace cafe
{
void applyUpgradesSystem()
{
    const float tapInterval =
        upgradeValue(UpgradeId::TapSpeed, UpgradeState::level(UpgradeId::TapSpeed));
    const float heat =
        upgradeValue(UpgradeId::MicrowaveSpeed, UpgradeState::level(UpgradeId::MicrowaveSpeed));

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (e.has<LiquidSpawner>()) e.get<LiquidSpawner>().interval = tapInterval;
        if (e.has<Microwave>())     e.get<Microwave>().heatTime     = heat;
    }
}
} // namespace cafe
