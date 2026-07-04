#include "SpawnButton.h"

#include <box2d/box2d.h>

namespace cafe
{
bool consumeSpawnRequest(SpawnButton& button)
{
    if (!button.justPressed) return false;
    button.justPressed = false;

    // slot occupied by another object -> ignore this spawn request
    b2ShapeId overlaps[1];
    return b2Shape_GetSensorOverlaps(button.spawnSlotSensor, overlaps, 1) == 0;
}
} // namespace cafe
