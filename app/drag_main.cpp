#include "Assets.h"
#include "Components.h"
#include "Entities.h"
#include "GameConfig.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "RenderContext.h"
#include "Systems.h"
#include "Transform.h"
#include "Utils.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include <iostream>
#include <optional>
#include <vector>

namespace
{
constexpr int kMachineRenderLayer = 0;
constexpr int kCupRenderLayer     = 1;

bool sameEntity(std::optional<bagel::ent_type> lhs, std::optional<bagel::ent_type> rhs)
{
    if (lhs.has_value() != rhs.has_value()) return false;
    if (!lhs.has_value()) return true;
    return lhs->id == rhs->id;
}

const char* dropTypeName(cafe::DropType dropType)
{
    switch (dropType)
    {
    case cafe::DropType::Any:
        return "Any";
    }
    return "Unknown";
}

void setBodySensorEvents(b2BodyId body, bool enabled)
{
    if (!b2Body_IsValid(body)) return;
    const int count = b2Body_GetShapeCount(body);
    if (count <= 0) return;
    std::vector<b2ShapeId> shapes(static_cast<size_t>(count));
    b2Body_GetShapes(body, shapes.data(), count);
    for (b2ShapeId shapeId : shapes)
        b2Shape_EnableSensorEvents(shapeId, enabled);
}

void enableSensorEventsOnHeldEntities()
{
    static const bagel::Mask heldMask = bagel::MaskBuilder().set<cafe::Held>().build();
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(heldMask)) continue;
        if (!e.has<cafe::PhysicsBody>()) continue;
        setBodySensorEvents(e.get<cafe::PhysicsBody>().id, true);
    }
}

void addDraggableVisitorShape(b2BodyId body, float halfW, float halfH)
{
    b2ShapeDef visitor = b2DefaultShapeDef();
    visitor.filter.categoryBits    = cafe::filter::DRAGGABLE;
    visitor.filter.maskBits        = cafe::filter::MASK_DRAGGABLE;
    visitor.enableSensorEvents     = true;
    b2Polygon box = b2MakeOffsetBox(halfW, halfH, { 0.f, 0.f }, b2Rot_identity);
    b2CreatePolygonShape(body, &visitor, &box);
}

void logEntityCreation(const char* label, bagel::ent_type id, cafe::WorldPos pos, Uint64 ms)
{
    std::cout << "[DragTest] Created " << label << " entity=" << id.id
              << " at world (" << pos.x << ", " << pos.y << ") t=" << ms << "ms\n";
}

void logHeldDebugState(Uint64 nowMs)
{
    static const bagel::Mask heldMask = bagel::MaskBuilder().set<cafe::Held>().build();
    static std::optional<bagel::ent_type> lastHeldEntity;
    static std::optional<bagel::ent_type> lastDropSpaceEntity;
    static Uint64 nextHeldLogMs = 0;

    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(heldMask)) continue;

        const auto& held = e.get<cafe::Held>();
        const std::optional<bagel::ent_type> currentHeldEntity = e.entity();
        const std::optional<bagel::ent_type> currentDropSpaceEntity = held.dropSpaceEntity;

        if (!sameEntity(lastHeldEntity, currentHeldEntity))
        {
            std::cout << "[DragTest] Holding entity=" << e.entity().id
                      << " dropType=" << dropTypeName(held.dropType) << "\n";
            nextHeldLogMs = nowMs + 1000;
        }

        if (!sameEntity(lastDropSpaceEntity, currentDropSpaceEntity))
        {
            if (lastDropSpaceEntity.has_value() && currentDropSpaceEntity.has_value())
            {
                std::cout << "[DragTest] Held entity=" << e.entity().id
                          << " moved from DropSpace entity=" << lastDropSpaceEntity->id
                          << " to DropSpace entity=" << currentDropSpaceEntity->id << "\n";
            }
            else if (currentDropSpaceEntity.has_value())
            {
                std::cout << "[DragTest] Held entity=" << e.entity().id
                          << " entered DropSpace entity=" << currentDropSpaceEntity->id
                          << " (release now would snap)\n";
            }
            else
            {
                std::cout << "[DragTest] Held entity=" << e.entity().id
                          << " left DropSpace entity=" << lastDropSpaceEntity->id
                          << " (release now would drop normally)\n";
            }
        }

        if (nowMs >= nextHeldLogMs)
        {
            std::cout << "[DragTest] Still holding entity=" << e.entity().id;
            if (e.has<cafe::Transform>())
            {
                const auto& t = e.get<cafe::Transform>();
                std::cout << " world (" << t.x << ", " << t.y << ")";
            }
            if (currentDropSpaceEntity.has_value())
            {
                std::cout << " over DropSpace entity=" << currentDropSpaceEntity->id;
            }
            else
            {
                std::cout << " over no DropSpace";
            }
            std::cout << " dropType=" << dropTypeName(held.dropType)
                      << " t=" << nowMs << "ms\n";
            nextHeldLogMs = nowMs + 1000;
        }

        lastHeldEntity = currentHeldEntity;
        lastDropSpaceEntity = currentDropSpaceEntity;
        return;
    }

    if (lastHeldEntity.has_value())
    {
        std::cout << "[DragTest] Holding ended for entity=" << lastHeldEntity->id
                  << " t=" << nowMs << "ms\n";
    }

    lastHeldEntity = std::nullopt;
    lastDropSpaceEntity = std::nullopt;
    nextHeldLogMs = 0;
}

SDL_FPoint mouseWindowToRenderPoint(float windowX, float windowY)
{
    SDL_FPoint renderPos{ windowX, windowY };
    SDL_RenderCoordinatesFromWindow(cafe::RenderContext::getRenderer(),
                                    windowX,
                                    windowY,
                                    &renderPos.x,
                                    &renderPos.y);
    return renderPos;
}

bagel::Entity createDropSpace(cafe::WorldPos pos)
{
    using namespace cafe;

    const float halfW = Assets::machineW() / (2.f * PTM);
    const float halfH = Assets::machineH() / (2.f * PTM);

    auto ent = bagel::Entity::create();
    Transform t{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH };

    b2BodyDef bd = b2DefaultBodyDef();
    bd.type     = b2_staticBody;
    bd.position = { t.x, t.y };
    bd.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    b2BodyId body = b2CreateBody(PhysicsContext::world(), &bd);

    b2ShapeDef sensor = b2DefaultShapeDef();
    sensor.isSensor               = true;
    sensor.enableSensorEvents     = true;
    sensor.filter.categoryBits    = filter::DROPSPACE_SENSOR;
    sensor.filter.maskBits        = filter::MASK_DROPSPACE_SENSOR;
    b2Polygon zone = b2MakeOffsetBox(halfW, halfH, { 0.f, 0.f }, b2Rot_identity);
    b2CreatePolygonShape(body, &sensor, &zone);

    ent.addAll(
        t,
        Drawable{ Assets::machine(), Assets::machineSrcRect(), kMachineRenderLayer },
        PhysicsBody{ body },
        DropSpace{ DropType::Any }
    );
    return ent;
}
} // namespace

int main()
{
    using namespace cafe;

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Init error : " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Window*   window{};
    SDL_Renderer* renderer{};
    SDL_CreateWindowAndRenderer("Drag and Drop Test",
                                static_cast<int>(START_WIN_W),
                                static_cast<int>(START_WIN_H),
                                SDL_WINDOW_OPENGL,
                                &window,
                                &renderer);
    assertFatal(window != nullptr, SDL_GetError());
    assertFatal(renderer != nullptr, SDL_GetError());

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    RenderContext::init(window, renderer);
    Assets::init(renderer);
    PhysicsContext::init();

    SDL_SetTextureScaleMode(Assets::cup(),     SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(Assets::machine(), SDL_SCALEMODE_NEAREST);

    std::cout << "[DragTest] Harness started — drag the cup onto the machine drop zone.\n"
              << "[DragTest] Systems: dropSpacePickupSystem (re-enable DropSpace sensors),\n"
              << "[DragTest]          dragStartSystem (pick), midDragSystem (follow mouse),\n"
              << "[DragTest]          dropSpaceDetectionSystem (sensor overlap), dragReleaseSystem (snap/drop).\n"
              << "[DragTest] Controls: left mouse drag; close window to quit.\n";

    auto cupEnt = createCup({ -3.f, 0.f }, 50);
    cupEnt.add(Draggable{ DropType::Any });
    cupEnt.get<Drawable>().renderLayer = kCupRenderLayer;

    const auto& cupTransform = cupEnt.get<Transform>();
    addDraggableVisitorShape(cupEnt.get<PhysicsBody>().id, cupTransform.w, cupTransform.h);
    logEntityCreation("cup (Draggable)", cupEnt.entity(), { cupTransform.x, cupTransform.y }, SDL_GetTicks());

    const WorldPos dropPos{ 3.f, 0.f };
    auto           dropEnt = createDropSpace(dropPos);
    logEntityCreation("DropSpace (machine)", dropEnt.entity(), dropPos, SDL_GetTicks());

    bool   isDragging = false;
    bool   isRunning  = true;
    Uint64 lastTicks  = SDL_GetTicks();
    while (isRunning)
    {
        const auto frameStart = SDL_GetTicks();
        float dt = static_cast<float>(frameStart - lastTicks) * 0.001f;
        if (dt > 0.05f) dt = 0.05f;
        lastTicks = frameStart;

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                isRunning = false;
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    const SDL_FPoint rawPos{ static_cast<float>(event.button.x),
                                             static_cast<float>(event.button.y) };
                    const SDL_FPoint pos = mouseWindowToRenderPoint(rawPos.x, rawPos.y);

                    dropSpacePickupSystem(pos);
                    dragStartSystem(pos);
                    enableSensorEventsOnHeldEntities();

                    bool picked = false;
                    static const bagel::Mask heldMask =
                        bagel::MaskBuilder().set<Held>().build();
                    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
                    {
                        if (!e.test(heldMask)) continue;
                        const WorldPos worldPos =
                            screenToWorldPoint(pos, RenderContext::getCameraPos());
                        std::cout << "[DragTest] Drag started — entity=" << e.entity().id
                                  << " screen (" << pos.x << ", " << pos.y << ")"
                                  << " world (" << worldPos.x << ", " << worldPos.y << ")\n";
                        picked = true;
                        break;
                    }

                    if (!picked)
                    {
                        std::cout << "[DragTest] Click — no draggable hit at screen ("
                                  << pos.x << ", " << pos.y << ")\n";
                    }

                    isDragging = true;
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (isDragging)
                {
                    const SDL_FPoint rawPos{ static_cast<float>(event.motion.x),
                                             static_cast<float>(event.motion.y) };
                    const SDL_FPoint pos = mouseWindowToRenderPoint(rawPos.x, rawPos.y);
                    midDragSystem(pos);
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button == SDL_BUTTON_LEFT && isDragging)
                {
                    static const bagel::Mask heldMask =
                        bagel::MaskBuilder().set<Held>().build();
                    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
                    {
                        if (!e.test(heldMask)) continue;
                        const auto& held = e.get<Held>();
                        if (held.dropSpaceEntity.has_value())
                        {
                            std::cout << "[DragTest] Release — snap entity=" << e.entity().id
                                      << " to DropSpace entity=" << held.dropSpaceEntity->id << "\n";
                        }
                        else
                        {
                            std::cout << "[DragTest] Release — entity=" << e.entity().id
                                      << " dropped (no valid DropSpace)\n";
                        }
                        break;
                    }

                    dragReleaseSystem();
                    isDragging = false;
                }
                break;
            }
        }

        PhysicsContext::step(dt);
        dropSpaceDetectionSystem();
        logHeldDebugState(SDL_GetTicks());
        syncTransformFromBody();

        SDL_RenderClear(renderer);
        drawSystem();
        SDL_RenderPresent(renderer);

        constexpr Uint64 frameDeltaT = static_cast<Uint64>(FRAME_DELTA_MS);
        auto             frameEnd    = SDL_GetTicks();
        if (frameEnd - frameStart < frameDeltaT)
            SDL_Delay(static_cast<Uint32>(frameDeltaT - (frameEnd - frameStart)));
    }

    PhysicsContext::shutdown();
    Assets::shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
