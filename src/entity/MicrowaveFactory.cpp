#include "MicrowaveFactory.h"

#include "AssetManager.h"
#include "Components.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "RenderLayers.h"
#include "SpriteDims.h"
#include <box2d/box2d.h>

namespace cafe
{
bagel::Entity createMicrowave(AssetManager& assets, PhysicsContext& physics, WorldPos pos)
{
    auto& spriteSheet = assets.getSpriteSheet(OVEN_TEX, OVEN_SPRITE_DATA);
    constexpr float halfW = texToWorldScale(OVEN_DIMS.x); // world half-extents — tune freely
    constexpr float halfH = texToWorldScale(OVEN_DIMS.y);

    auto ent = bagel::Entity::create();

    // Static body carrying one isSensor box, so a held pat's draggable visitor
    // shape triggers begin/end events that dropSpaceDetectionSystem reads.
    b2BodyDef bd = b2DefaultBodyDef();
    bd.type      = b2_staticBody;
    bd.position  = { pos.x, pos.y };
    bd.userData  = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    b2BodyId body = b2CreateBody(physics.world(), &bd);

    b2ShapeDef sensor          = b2DefaultShapeDef();
    sensor.isSensor            = true;
    sensor.enableSensorEvents  = true;
    sensor.filter.categoryBits = filter::DROPSPACE_SENSOR;
    sensor.filter.maskBits     = filter::MASK_DROPSPACE_SENSOR; // == DRAGGABLE
    b2Polygon zone = b2MakeOffsetBox(halfW, halfH, { 0.f, 0.f }, b2Rot_identity);
    b2CreatePolygonShape(body, &sensor, &zone);

    ent.addAll(
        Transform{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH },
        Drawable{
            assets.getTexture(OVEN_TEX).get(),
            spriteSheet.getFrameRect(OVEN_DEFAULT_SPRITE_ID),
            layer::STATIC_ON_BARTOP
        },
        PhysicsBody{ body },
        DropSpace{ DropType::Pastry },
        Microwave{}
    );

    return ent;
}
} // namespace cafe
