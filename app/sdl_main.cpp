#include "AssetManager.h"
#include "Assets.h"
#include "Components.h"
#include "Entities.h"
#include "GameConfig.h"
#include "PhysicsContext.h"
#include "PhysicsFilters.h"
#include "RenderContext.h"
#include "Systems.h"
#include "Utils.h"
#include <SDL3/SDL.h>
#include <box2d/box2d.h>
#include <iostream>
#include <vector>

namespace
{
// Speech-bubble geometry (measured from bubble.png, same as jonathan_main).
constexpr float      BUBBLE_DISPLAY_W      = 24.f;          // logical px
constexpr float      BUBBLE_DISPLAY_H      = 14.f;
constexpr SDL_FPoint BUBBLE_TAIL_OFFSET_PX = { 7.5f, -6.5f };
constexpr SDL_FPoint CUSTOMER_MOUTH_OFFSET = { -4.f, -1.3f };

// Order-icon layout inside the bubble.
constexpr float ICON_SIZE = 8.f / 1.5f; // ~5.33 logical px, square
constexpr float ICON_DX   = 3.f;
constexpr float ICON_DY   = 2.f;

// How many drops fill the cup.
constexpr int kCupCapacity = 50;

// ---- helper functions -------------------------------------------------------

SDL_FPoint mouseWindowToRenderPoint(float windowX, float windowY)
{
    SDL_FPoint p{ windowX, windowY };
    SDL_RenderCoordinatesFromWindow(cafe::RenderContext::getRenderer(),
                                    windowX, windowY, &p.x, &p.y);
    return p;
}

void setBodySensorEvents(b2BodyId body, bool enabled)
{
    if (!b2Body_IsValid(body)) return;
    const int count = b2Body_GetShapeCount(body);
    if (count <= 0) return;
    std::vector<b2ShapeId> shapes(static_cast<size_t>(count));
    b2Body_GetShapes(body, shapes.data(), count);
    for (b2ShapeId s : shapes)
        b2Shape_EnableSensorEvents(s, enabled);
}

void enableSensorEventsOnHeldEntities()
{
    static const bagel::Mask mask = bagel::MaskBuilder().set<cafe::Held>().build();
    for (auto e = bagel::Entity::first(); !e.eof(); e.next())
    {
        if (!e.test(mask)) continue;
        if (!e.has<cafe::PhysicsBody>()) continue;
        setBodySensorEvents(e.get<cafe::PhysicsBody>().id, true);
    }
}

// Adds a DRAGGABLE sensor shape to an existing physics body so the drag system
// can hit-test it via Box2D sensor events.
void addDraggableVisitorShape(b2BodyId body, float halfW, float halfH)
{
    b2ShapeDef def          = b2DefaultShapeDef();
    def.isSensor            = true;
    def.enableSensorEvents  = true;
    def.filter.categoryBits = cafe::filter::DRAGGABLE;
    def.filter.maskBits     = cafe::filter::MASK_DRAGGABLE;
    b2Polygon box = b2MakeOffsetBox(halfW, halfH, { 0.f, 0.f }, b2Rot_identity);
    b2CreatePolygonShape(body, &def, &box);
}

// Creates a kinematic pastry entity sitting on the counter, draggable.
bagel::Entity createPastry(cafe::WorldPos pos, SDL_Texture* propsTex,
                           float propsW, float propsH)
{
    using namespace cafe;

    const float halfW = (propsW / 3.f) / (2.f * PTM);
    const float halfH = propsH / (2.f * PTM);

    auto ent = bagel::Entity::create();

    b2BodyDef bd = b2DefaultBodyDef();
    bd.type      = b2_kinematicBody;
    bd.position  = { pos.x, pos.y };
    bd.userData  = reinterpret_cast<void*>(static_cast<uintptr_t>(ent.entity().id));
    b2BodyId body = b2CreateBody(PhysicsContext::world(), &bd);
    addDraggableVisitorShape(body, halfW, halfH);

    // Frame 0 of the 3-frame props strip = cinnamon roll.
    SDL_FRect src = { 0.f, 0.f, propsW / 3.f, propsH };
    ent.addAll(
        Transform{ .x = pos.x, .y = pos.y, .w = halfW, .h = halfH },
        Drawable{ propsTex, src, kLayerPastry },
        PhysicsBody{ body },
        Draggable{ DropType::Any }
    );
    return ent;
}

// Attaches a static DropSpace sensor + Served to an existing client entity so
// dragged items can be "dropped on" the customer.
void makeCustomerDeliverable(bagel::Entity client)
{
    using namespace cafe;

    const auto& t = client.get<Transform>();

    b2BodyDef bd = b2DefaultBodyDef();
    bd.type      = b2_staticBody;
    bd.position  = { t.x, t.y };
    bd.userData  = reinterpret_cast<void*>(static_cast<uintptr_t>(client.entity().id));
    b2BodyId body = b2CreateBody(PhysicsContext::world(), &bd);

    b2ShapeDef sensor          = b2DefaultShapeDef();
    sensor.isSensor            = true;
    sensor.enableSensorEvents  = true;
    sensor.filter.categoryBits = filter::DROPSPACE_SENSOR;
    sensor.filter.maskBits     = filter::MASK_DROPSPACE_SENSOR;
    b2Polygon zone = b2MakeOffsetBox(t.w, t.h, { 0.f, 0.f }, b2Rot_identity);
    b2CreatePolygonShape(body, &sensor, &zone);

    client.addAll(
        PhysicsBody{ body },
        DropSpace{ DropType::Any },
        Served{}
    );
}

} // anonymous namespace

int main()
{
    {

        auto cupEnt = createCup({ -4.f, -1.f }, Assets::cup(), Assets::cupW(), Assets::cupH(), kCupCapacity);
        cupEnt.get<Drawable>().renderLayer = kLayerCup;
        // Make the cup draggable.
        cupEnt.add(Draggable{ DropType::Any });
        {
            const auto& ct = cupEnt.get<Transform>();
            addDraggableVisitorShape(cupEnt.get<PhysicsBody>().id, ct.w, ct.h);
        }

        // Cleanup zone: off-screen sensor destroys spilled drops.
        createCleanupZone();

        // --- Pastry (draggable, sits right-side of counter) ---
        auto pastryEnt = createPastry({ 4.f, -3.f }, propsTex, propsW, propsH);
        (void)pastryEnt; // referenced only by the ECS from here on

        // --- Customer ---
        const Order clientOrder{ .ratio = {3, 7, 0}, .hasDrink = true, .hasPastry = true };
        auto customerEnt = createClient(customerTex, customerW, customerH,
                                        { 3.f, -1.f }, clientOrder, 60.f,
                                        CUSTOMER_MOUTH_OFFSET);
        customerEnt.get<Drawable>().renderLayer = kLayerClient;
        makeCustomerDeliverable(customerEnt); // adds DropSpace + Served

        // --- Speech bubble + order icons (children of customer) ---
        {
            const SDL_FPoint mouth = customerEnt.get<SpeechAnchor>().mouthOffset;
            const SDL_FPoint bubbleOff = { mouth.x - BUBBLE_TAIL_OFFSET_PX.x,
                                           mouth.y - BUBBLE_TAIL_OFFSET_PX.y + 1.f };
            const SDL_FRect  bubbleSrc = { 0.f, 0.f, bubbleW, bubbleH };
            auto bubble = createSpeechBubble(bubbleTex, bubbleSrc,
                                             BUBBLE_DISPLAY_W, BUBBLE_DISPLAY_H,
                                             customerEnt, bubbleOff);
            bubble.get<Drawable>().renderLayer = kLayerBubble;

            // Pastry icon (left) — frame 0 of props strip.
            const SDL_FRect pastrySrc  = { 0.f, 0.f, propsW / 3.f, propsH };
            auto pastryIcon = createOrderIcon(propsTex, pastrySrc,
                                              ICON_SIZE, ICON_SIZE,
                                              bubble, { -ICON_DX, ICON_DY });
            pastryIcon.get<Drawable>().renderLayer = kLayerIcon;

            // Drink icon (right) — front frame of big_cup (24x24).
            const SDL_FRect drinkSrc = { 0.f, 0.f, 24.f, 24.f };
            auto drinkIcon = createOrderIcon(bigCupTex, drinkSrc,
                                             ICON_SIZE, ICON_SIZE,
                                             bubble, { ICON_DX, ICON_DY });
            drinkIcon.get<Drawable>().renderLayer = kLayerIcon;
        }

        std::cout << "[Demo] Hold SPACE to pour coffee. Left-drag to move the cup or pastry.\n"
                  << "[Demo] Serve a FULL cup + pastry to the customer in 60 s.\n";

        bool   isRunning  = true;
        bool   isDragging = false;
        Uint64 lastTicks  = SDL_GetTicks();

        while (isRunning)
        {
            const Uint64 frameStart = SDL_GetTicks();
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

                case SDL_EVENT_KEY_DOWN:
                    if (event.key.scancode == SDL_SCANCODE_SPACE
                        && !machineEnt.get<CoffeeSpawner>().active)
                    {
                        machineEnt.get<CoffeeSpawner>().active = true;
                        std::cout << "[Pour] ON\n";
                    }
                    break;

                case SDL_EVENT_KEY_UP:
                    if (event.key.scancode == SDL_SCANCODE_SPACE
                        && machineEnt.get<CoffeeSpawner>().active)
                    {
                        machineEnt.get<CoffeeSpawner>().active = false;
                        std::cout << "[Pour] OFF\n";
                    }
                    break;

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    if (event.button.button == SDL_BUTTON_LEFT)
                    {
                        const SDL_FPoint pos = mouseWindowToRenderPoint(
                            static_cast<float>(event.button.x),
                            static_cast<float>(event.button.y));
                        dropSpacePickupSystem(pos);
                        dragStartSystem(pos);
                        enableSensorEventsOnHeldEntities();
                        isDragging = true;
                    }
                    break;

                case SDL_EVENT_MOUSE_MOTION:
                    if (isDragging)
                    {
                        const SDL_FPoint pos = mouseWindowToRenderPoint(
                            static_cast<float>(event.motion.x),
                            static_cast<float>(event.motion.y));
                        midDragSystem(pos);
                    }
                    break;

                case SDL_EVENT_MOUSE_BUTTON_UP:
                    if (event.button.button == SDL_BUTTON_LEFT && isDragging)
                    {
                        // deliverySystem reads Held.dropSpaceEntity before
                        // dragReleaseSystem removes the Held component.
                        deliverySystem();
                        dragReleaseSystem();
                        isDragging = false;
                    }
                    break;
                }
            }

            coffeeSpawnerSystem(dt, Assets::particle());    // spawn drops while pouring
            PhysicsContext::step(dt);
            sensorEventSystem();        // count drops into cup; cleanup spilled
            dropSpaceDetectionSystem(); // update Held.dropSpaceEntity
            syncTransformFromBody();    // physics position -> Transform

            behaviorSystem(dt);         // tick patience; adds Leaving on timeout (fail)
            orderSystem();              // full cup + pastry -> rating=1 + Leaving (success)
            reportLeavingClients();     // log SUCCESSFUL / FAILED
            hierarchySystem();          // children follow parents; orphan children of Leaving
            cleanupSystem();            // destroy all Leaving entities

            SDL_RenderClear(renderer);
            drawSystem(renderer);       // sorted by renderLayer ascending
            SDL_RenderPresent(renderer);

            constexpr Uint64 frameDeltaT = static_cast<Uint64>(FRAME_DELTA_MS);
            const    Uint64  frameEnd    = SDL_GetTicks();
            if (frameEnd - frameStart < frameDeltaT)
                SDL_Delay(static_cast<Uint32>(frameDeltaT - (frameEnd - frameStart)));
        }

    } // AssetManager freed here — before renderer is destroyed.

    PhysicsContext::shutdown();
    Assets::shutdown();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}