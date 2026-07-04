#pragma once

namespace cafe
{
enum class DropType
{
    Any = 0,
    Cup,
    Pastry,
    Ice
};

static_assert(static_cast<int>(DropType::Ice) == 3, "Only 4 drop types supported"); // only 4 supported
} // namespace cafe
