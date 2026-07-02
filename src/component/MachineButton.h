#pragma once
#include "Ingredient.h"
#include <functional>

namespace cafe
{
struct MachineButton
{
    LiquidIngredient kind{LiquidIngredient::count};
    bool pressed{};
};
}

template <> struct bagel::Storage<cafe::MachineButton> final : NoInstance { using type = StackStorage<cafe::MachineButton>; };