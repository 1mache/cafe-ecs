#include "MicrowaveFactory.h"

#include "AssetManager.h"
#include "Components.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "RenderLayers.h"
#include "Texture.h"
#include <box2d/box2d.h>

namespace cafe
{
bagel::Entity createMicrowave(AssetManager& assets, PhysicsContext& physics, WorldPos pos)
{
    // particle.png is reused only to give Drawable a valid texture handle;
    // drawSystem renders a Microwave as a solid square (placeholder art).
    static constexpr auto TEX = "particle.png";
    const Texture& tex = assets.getTexture(TEX);

    constexpr float halfW = 1.2f; // world half-extents — tune freely
    constexpr float halfH = 1.4f;

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
        Drawable{ tex.get(), tex.getFullSrcRect(), layer::STATIC_ON_BARTOP },
        PhysicsBody{ body },
        DropSpace{ DropType::Pastry },
        Microwave{}
    );

    // Heating-progress bar as its own child entity (placeholder: tinted particle.png).
    // Starts at width 0 (hidden); microwaveBarSystem grows it from the machine's timer.
    auto bar = bagel::Entity::create();
    bar.addAll(
        Transform{ .x = pos.x, .y = pos.y, .w = 0.f, .h = BAR_HEIGHT },
        Drawable{ tex.get(), tex.getFullSrcRect(), layer::UI1, SDL_Color{ 240, 180, 40, 255 } },
        TimerBar{ .source = ent }
    );
    return ent;
}
} // namespace cafe
