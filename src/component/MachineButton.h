#pragma once
#include "Ingredient.h"
#include <functional>

namespace cafe
{
struct MachineButton
{
    LiquidIngredient kind{LiquidIngredient::count};
    bool pressed{};
    bool loopAction{}; // whether we want to loop onClick() as long as pressed.
};
}

template <> struct bagel::Storage<cafe::MachineButton> final : NoInstance { using type = StackStorage<cafe::MachineButton>; };