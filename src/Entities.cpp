#include "Entities.h"
#include "Assets.h"
#include "GameConfig.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include <box2d/box2d.h>
#include <cstdint>

namespace cafe
{
// Cup geometry shared between the SDL-driven and headless factories.
// Three solid wall shapes + one interior sensor on a single kinematic body.
static bagel::Entity createCupCommon(WorldPos pos, float halfW, float halfH,
                                     int capacity, SDL_Texture* tex, SDL_FRect src)
{
    auto ent = bagel::Entity::create();
    const float wallT = 0.5f / PTM; // wall thickness in world units (~half a pixel)
    Transform t{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH };

    b2BodyDef bd = b2DefaultBodyDef();
    bd.type     = b2_kinematicBody; // kinematic so drag-and-drop can move the cup later
    bd.position = { t.x, t.y };
    bd.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    b2BodyId body = b2CreateBody(PhysicsContext::world(), &bd);

    // Solid walls + bottom — stop drops from passing through.
    b2ShapeDef wall = b2DefaultShapeDef();
    wall.filter.categoryBits = filter::CUP_SOLID;
    wall.filter.maskBits     = filter::MASK_CUP_SOLID;
    b2Polygon leftWall  = b2MakeOffsetBox(wallT, halfH, { -halfW + wallT, 0.f }, b2Rot_identity);
    b2Polygon rightWall = b2MakeOffsetBox(wallT, halfH, {  halfW - wallT, 0.f }, b2Rot_identity);
    b2Polygon bottom    = b2MakeOffsetBox(halfW, wallT, { 0.f, -halfH + wallT }, b2Rot_identity);
    b2CreatePolygonShape(body, &wall, &leftWall);
    b2CreatePolygonShape(body, &wall, &rightWall);
    b2CreatePolygonShape(body, &wall, &bottom);

    // Interior sensor — fires the begin-contact that counts a fill.
    // enableSensorEvents must be set explicitly (Box2D 3.x default is false).
    b2ShapeDef sensor = b2DefaultShapeDef();
    sensor.isSensor            = true;
    sensor.enableSensorEvents  = true;
    sensor.filter.categoryBits = filter::CUP_INSIDE;
    sensor.filter.maskBits     = filter::MASK_CUP_INSIDE;
    b2Polygon interior = b2MakeOffsetBox(halfW - wallT, halfH - wallT,
                                         { 0.f, wallT * 0.5f }, b2Rot_identity);
    b2CreatePolygonShape(body, &sensor, &interior);

    ent.addAll(
        t,
        Drawable{ tex, src },
        PhysicsBody{ body },
        Cup{ .capacity = capacity }
    );
    return ent;
}

bagel::Entity createLiquidDrop(WorldPos pos)
{
    auto ent = bagel::Entity::create();
    constexpr float r = 0.06f; // ~0.5 px radius. Raise to 0.10f if drops jitter.

    b2BodyDef bd = b2DefaultBodyDef();
    bd.type     = b2_dynamicBody;
    bd.position = { pos.x, pos.y };
    bd.isBullet = true; // CCD — prevents tunneling through the thin cup walls
    bd.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    b2BodyId body = b2CreateBody(PhysicsContext::world(), &bd);

    // Visitor side of a sensor pair must also opt in to sensor events.
    b2ShapeDef sd = b2DefaultShapeDef();
    sd.density             = 1.f;
    sd.material.friction   = 0.1f;
    sd.enableSensorEvents  = true;
    sd.filter.categoryBits = filter::LIQUID;
    sd.filter.maskBits     = filter::MASK_LIQUID;

    b2Circle circle{ {0.f, 0.f}, r };
    b2CreateCircleShape(body, &sd, &circle);

    // Assets::particle() may be null in unit tests — fall back to a 2x2 stub rect.
    SDL_Texture* tex = Assets::particle();
    SDL_FRect    src = tex ? Assets::particleSrcRect() : SDL_FRect{ 0.f, 0.f, 2.f, 2.f };

    ent.addAll(
        Liquid{},
        Transform{ .x = pos.x, .y = pos.y, .w = r, .h = r },
        Drawable{ tex, src },
        PhysicsBody{ body }
    );
    return ent;
}

bagel::Entity createCup(WorldPos pos, int capacity)
{
    const float halfW = Assets::cupW() / (2.f * PTM);
    const float halfH = Assets::cupH() / (2.f * PTM);
    return createCupCommon(pos, halfW, halfH, capacity,
                           Assets::cup(), Assets::cupSrcRect());
}

bagel::Entity createCupHeadless(WorldPos pos, float texW, float texH, int capacity)
{
    const float halfW = texW / (2.f * PTM);
    const float halfH = texH / (2.f * PTM);
    return createCupCommon(pos, halfW, halfH, capacity, nullptr, { 0.f, 0.f, texW, texH });
}

bagel::Entity createCoffeeMachine(WorldPos pos, WorldPos spoutOffset)
{
    auto ent = bagel::Entity::create();

    const float halfW = Assets::machineW() / (2.f * PTM);
    const float halfH = Assets::machineH() / (2.f * PTM);
    Transform t{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH };

    // Kinematic body, no fixtures — the machine is just a position anchor for spawning.
    b2BodyDef bd = b2DefaultBodyDef();
    bd.type     = b2_kinematicBody;
    bd.position = { t.x, t.y };
    bd.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    b2BodyId body = b2CreateBody(PhysicsContext::world(), &bd);

    ent.addAll(
        t,
        Drawable{ Assets::machine(), Assets::machineSrcRect() },
        PhysicsBody{ body },
        CoffeeSpawner{
            .interval    = 0.05f,
            .accumulator = 0.f,
            .active      = false,
            .offset      = spoutOffset
        }
    );
    return ent;
}

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

void destroyPhysicalEntity(bagel::ent_type id)
{
    bagel::Entity e{ id };
    if (e.has<PhysicsBody>())
        b2DestroyBody(e.get<PhysicsBody>().id);
    e.destroy();
}
} // namespace cafe
