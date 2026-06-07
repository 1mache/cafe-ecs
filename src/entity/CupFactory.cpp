#include "CupFactory.h"
#include "Assets.h"
#include "Components.h"
#include "GameConfig.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include <box2d/box2d.h>

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
} // namespace cafe
