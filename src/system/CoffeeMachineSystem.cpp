#include "CoffeeMachineSystem.h"

#include "Components.h"
#include "bagel.h"

namespace cafe
{
void machineButtonSystem()
{
    static const bagel::Mask buttonMask  = bagel::MaskBuilder().set<MachineButton>().build();
    static const bagel::Mask spawnerMask = bagel::MaskBuilder().set<LiquidSpawner>().build();

    std::array<bool, static_cast<std::size_t>(LiquidIngredient::count)> currentlyPressed = {};

    for (auto btn = bagel::Entity::first(); !btn.eof(); btn.next())
    {
        if (!btn.test(buttonMask)) continue;

        const auto& button = btn.get<MachineButton>();
        currentlyPressed[static_cast<size_t>(button.kind)] = button.pressed;
    }

    for (auto sp = bagel::Entity::first(); !sp.eof(); sp.next())
    {
        if (!sp.test(spawnerMask)) continue;
        auto& spawner = sp.get<LiquidSpawner>();
        spawner.active = currentlyPressed[static_cast<size_t>(spawner.kind)];
    }
}
}