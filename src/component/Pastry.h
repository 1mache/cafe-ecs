#pragma once
#include "ItemTypes.h"

namespace cafe
{
struct Pastry
{
    PastryType type{PastryType::count};
    // pastry cold by default
    Temperature temperature{Temperature::Cold};
};
}