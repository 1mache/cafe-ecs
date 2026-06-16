#pragma once
#include "box2d/id.h"

#include <bagel.h>

namespace cafe
{
    struct SpawnSensor
    {
        b2BodyId body{};
    };
}

template <> struct bagel::Storage<cafe::SpawnSensor> final : NoInstance {using type = SparseStorage<cafe::SpawnSensor>; };