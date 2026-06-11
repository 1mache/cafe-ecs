#pragma once

namespace cafe::layer
{
enum RenderLayer : int
{
    BG = 0,
    CUSTOMER,
    BARTOP,
    STATIC_ON_BARTOP, // coffee machine, pastry holder
    CONTAINER_BACK,
    LIQUID,
    CONTAINER_FRONT,
    PROP, // draggable objects
    UI1,
    UI2,
    COUNT // how many layers
};
}