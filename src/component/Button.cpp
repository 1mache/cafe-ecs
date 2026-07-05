#include "Button.h"

#include <box2d/box2d.h>

namespace cafe
{
bool consumeSpawnRequest(Button& button)
{
    if (!button.pressed) return false;
    button.pressed = false;

    // slot occupied by another object -> ignore this spawn request
    b2ShapeId overlaps[1];
    return b2Shape_GetSensorOverlaps(button.spawnSlotSensor, overlaps, 1) == 0;
}
} // namespace cafe
