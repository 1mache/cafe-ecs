#include "CleanupZoneFactory.h"
#include "Components.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include <box2d/box2d.h>

namespace cafe
{
bagel::Entity createCleanupZone()
{
    auto ent = bagel::Entity::create();

    // Static body just below the visible play area. Top of the sensor sits at
    // y = -6 (screen bottom is ~-5.625) so missed drops are cleaned up quickly
    // instead of living as physics bodies for a full second.
    b2BodyDef bd = b2DefaultBodyDef();
    bd.type     = b2_staticBody;
    bd.position = { 0.f, -8.f };
    bd.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    b2BodyId body = b2CreateBody(PhysicsContext::world(), &bd);

    b2ShapeDef sd = b2DefaultShapeDef();
    sd.isSensor            = true;
    sd.enableSensorEvents  = true;
    sd.filter.categoryBits = filter::CLEANUP;
    sd.filter.maskBits     = filter::MASK_CLEANUP;

    b2Polygon area = b2MakeBox(100.f, 2.f); // 200 m wide x 4 m tall
    b2CreatePolygonShape(body, &sd, &area);

    // Intentionally no Transform / no Drawable — invisible and immobile.
    ent.addAll(
        PhysicsBody{ body },
        CleanupZone{}
    );
    return ent;
}
} // namespace cafe
