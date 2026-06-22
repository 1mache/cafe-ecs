#include "CupFactory.h"
#include "AssetManager.h"
#include "Components.h"
#include "DragAndDropSystem.h"
#include "RenderLayers.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "Texture.h"
#include "SpriteDims.h"
#include <box2d/box2d.h>

namespace cafe
{
namespace
{
constexpr auto  TEX                    = "cup.png";

constexpr float WALL_W_PIX             = 2.f;
constexpr float WALL_H_PIX             = 17.f;
constexpr float L_WALL_X_OFFSET_PIX    = -1.f;
constexpr float R_WALL_X_OFFSET_PIX    = 7.f;
constexpr float CUP_WALLS_Y_OFFSET_PIX = 12.f;
constexpr float CUP_BOTTOM_Y_OFFSET_PIX= 2.f;
constexpr float BOTTOM_W_PIX           = 14.f;
constexpr float BOTTOM_L_OFFSET_PIX    = 2.f;

// tested how many particles fit at max without too much over the top
constexpr int   CUP_CAPACITY           = 250;

constexpr float CUP_FRICTION    = 0.1f;
constexpr float CUP_RESTITUTION = 0.1f;

} // namespace

bagel::Entity createCup(AssetManager& assets, PhysicsContext& physics, WorldPos pos)
{
    const Texture& tex = assets.getTexture(TEX);
    constexpr float halfW = screenToWorldScale(CUP_DIMS.x);
    constexpr float halfH = screenToWorldScale(CUP_DIMS.y);

    const float wallW     = screenToWorldDistance(WALL_W_PIX);
    const float wallHalfH = screenToWorldScale(WALL_H_PIX);
    const float wallHalfW = screenToWorldScale(WALL_W_PIX);
    const float leftWallX =
        (-halfW + screenToWorldDistance(L_WALL_X_OFFSET_PIX) + wallW);
    const float rightWallX =
        (halfW - screenToWorldDistance(R_WALL_X_OFFSET_PIX) - wallW);
    const float wallY       = -halfH + screenToWorldDistance(CUP_WALLS_Y_OFFSET_PIX);
    const float bottomHalfH = wallHalfW; // same thickness as walls
    const float bottomHalfW = screenToWorldScale(BOTTOM_W_PIX);
    const float bottomX     = (-halfW + screenToWorldDistance(BOTTOM_L_OFFSET_PIX) + bottomHalfW);
    const float bottomY =
        -halfH + wallW + screenToWorldDistance(CUP_BOTTOM_Y_OFFSET_PIX);

    Transform t{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH };

    auto cupBack  = bagel::Entity::create();
    auto cupFront = bagel::Entity::create();

    b2BodyDef bd = b2DefaultBodyDef();
    bd.type     = b2_dynamicBody;
    bd.position = { t.x, t.y };
    bd.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(cupBack.entity().id));
    bd.isBullet = true;
    bd.fixedRotation = true;
    b2BodyId body = b2CreateBody(physics.world(), &bd);

    // Solid walls + bottom — stop drops from passing through.
    b2ShapeDef wallShape = b2DefaultShapeDef();
    wallShape.material.friction = CUP_FRICTION;
    wallShape.material.restitution = CUP_RESTITUTION;
    wallShape.filter.categoryBits = filter::CUP_SOLID;
    wallShape.filter.maskBits     = filter::MASK_CUP_SOLID;
    b2Polygon leftWall  = b2MakeOffsetBox(wallHalfW, wallHalfH, { leftWallX, wallY }, b2Rot_identity);
    b2Polygon rightWall = b2MakeOffsetBox(wallHalfW, wallHalfH, { rightWallX, wallY }, b2Rot_identity);
    b2Polygon bottom    = b2MakeOffsetBox(bottomHalfW, bottomHalfH, { bottomX, bottomY }, b2Rot_identity);
    b2CreatePolygonShape(body, &wallShape, &leftWall);
    b2CreatePolygonShape(body, &wallShape, &rightWall);
    b2CreatePolygonShape(body, &wallShape, &bottom);

    // lid + guard that only collide with cup objects so cups can't go into each other
    b2ShapeDef lidShape = b2DefaultShapeDef();
    lidShape.material.friction = CUP_FRICTION;
    lidShape.material.restitution = CUP_RESTITUTION;
    lidShape.filter.categoryBits = filter::CUP_LID;
    lidShape.filter.maskBits     = filter::MASK_CUP_LID;
    b2Polygon lid = bottom; // same dimensions, only y position needs a tweak
    lid.centroid.y *= -1.f;
    b2Vec2 centerOfInsideCup = {-(rightWallX - leftWallX)/2 + (1.5f * wallW), wallW};
    b2Polygon insideGuard = b2MakeOffsetBox((rightWallX - leftWallX) / 2.f,
                                  wallHalfH,
                                  centerOfInsideCup,
                                  b2Rot_identity);
    b2CreatePolygonShape(body, &lidShape, &lid);
    b2CreatePolygonShape(body, &lidShape, &insideGuard);

    // Interior sensor — fires the begin-contact that counts a fill.
    b2ShapeDef sensor = b2DefaultShapeDef();
    sensor.isSensor            = true;
    sensor.enableSensorEvents  = true;
    sensor.filter.categoryBits = filter::CUP_INSIDE;
    sensor.filter.maskBits     = filter::MASK_CUP_INSIDE;
    // width is calculated based on wall positions and with + some epsilon
    // to make the sensor overlap with the walls a little bit
    b2Polygon interior = b2MakeOffsetBox((rightWallX - leftWallX - wallW)/2.f + 0.03f,
                                        wallHalfH,
                                         centerOfInsideCup,
                                         b2Rot_identity);
    b2CreatePolygonShape(body, &sensor, &interior);

    SDL_FRect frontSrcRect = {0, 0, CUP_DIMS.x, CUP_DIMS.y};
    SDL_FRect backSrcRect = {CUP_DIMS.x, 0, CUP_DIMS.x, CUP_DIMS.y};

    cupBack.addAll(
        t,
        Drawable{ tex.get(), backSrcRect, layer::CONTAINER_BACK },
        PhysicsBody{ body },
        Cup{ .capacity = CUP_CAPACITY },
        DragIntent{},
        DragItemType{ .dropType = DropType::Cup }
    );

    addDraggableVisitorShape(body, halfW, halfH);

    cupFront.addAll(
        Transform(t),
        Drawable{ tex.get(), frontSrcRect, layer::CONTAINER_FRONT },
        ChildOf(cupBack, {}));

    return cupBack;
}

} // namespace cafe
