#pragma once

namespace cafe::layer
{
enum RenderLayer : int
{
    BG = 0,
    CUSTOMER,
    BARTOP,
    STATIC_ON_BARTOP, // coffee machine, pastry holder
    STATIC_OVERLAY,   // labels/numbers/buttons drawn on top of static furniture
    CONTAINER_BACK,
    LIQUID,
    CONTAINER_FRONT,
    PROP, // draggable objects
    UI1,
    UI2,
    COUNT // how many layers
};
}