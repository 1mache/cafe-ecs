#include "CupFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "GameConfig.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "Texture.h"
#include "SpriteDims.h"
#include <box2d/box2d.h>

namespace cafe
{
// Cup geometry shared between the SDL-driven and headless factories.
// Three solid wall shapes + one interior sensor on a single kinematic body.
static bagel::Entity createCupCommon(WorldPos pos, int capacity, SDL_Texture* tex)
{
    const float halfW = screenToWorldScale(CUP_DIMS.x);
    const float halfH = screenToWorldScale(CUP_DIMS.y);

    // wall thickness in world units (1 pixel)
    const float wallT = screenToWorldScale(1.f);
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

    SDL_FRect frontSrcRect = {0, 0, CUP_DIMS.x, CUP_DIMS.y};
    SDL_FRect backSrcRect = {CUP_DIMS.x, 0, CUP_DIMS.x, CUP_DIMS.y};

    auto cupBack = bagel::Entity::create();
    auto cupFront = bagel::Entity::create();

    cupBack.addAll(
        t,
        Drawable{ tex, backSrcRect, LAYER_CONTAINER_BACK },
        PhysicsBody{ body },
        Cup{ .capacity = capacity },
        Draggable{ DropType::Any }
    );

    addDraggableVisitorShape(body, halfW, halfH);

    cupFront.addAll(
        Transform(t),
        Drawable{ tex, frontSrcRect, LAYER_CONTAINER_FRONT },
        ChildOf(cupBack, {}));

    return cupBack;
}

bagel::Entity createCup(AssetManager& assets, WorldPos pos, int capacity)
{
    static constexpr auto TEX = "big_cup.png";
    const Texture& tex = assets.getTexture(TEX);
    auto [w, h] = tex.getSize();
    return createCupCommon(pos, capacity, tex.get());
}

bagel::Entity createCupHeadless(WorldPos pos, float texW, float texH, int capacity)
{
    const float halfW = screenToWorldScale(CUP_DIMS.x);
    const float halfH = screenToWorldScale(CUP_DIMS.y);
    return createCupCommon(pos, capacity, nullptr);
}
} // namespace cafe
