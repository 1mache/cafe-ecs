#include "MainGameScene.h"
#include "Components.h"
#include "Entities.h"
#include "PhysicsContext.h"
#include "SpriteDims.h"
#include "Systems.h"

#include <iostream>

void cafe::MainGameScene::onInit()
{
    PhysicsContext::init();

    auto& assets = getAssetManager();
    createBg(assets, BG_PATH);
    createBartop(assets);

    _machineEnt = createCoffeeMachine(assets, {-6.f, -1.f}, {0.f, -0.5f});

    createCup(assets, {-4.f, -1.f}, CUP_CAPACITY);
    createPastry({4.f, -3.f},  assets);

    // Cleanup zone: off-screen sensor destroys spilled drops.
    createCleanupZone();

    std::cout << "[Demo] Hold SPACE to pour coffee. Left-drag to move the cup or pastry.\n"
              << "[Demo] Serve a FULL cup + pastry to the customer in 60 s.\n";

    // --- Customer ---
    Order customerOrder{ .ratio = {3, 7, 0}, .hasDrink = true, .hasPastry = true };
    auto customerEnt = createCustomer(assets, { 5.f, -1.f }, customerOrder, 60.f);

    // --- Speech bubble + order icons (children of customer) ---
    auto bubbleEnt = createSpeechBubble(assets, customerEnt, {-PERSON_DIMS.x, PERSON_DIMS.y * 0.25f});
    // --- Order icons (children of the bubble) ---
    constexpr float ICON_SIZE = 8.f / 1.5f; // TODO:move somewhere else
    constexpr float ICON_DX   = 3.f;
    constexpr float ICON_DY   = 2.f;
    if (customerOrder.hasDrink)
    {
        // Drink icon (right) — front frame of cup (16x16).
        createOrderIcon(assets,
                        2,
                        ICON_SIZE,
                        ICON_SIZE,
                        bubbleEnt,
                        {ICON_DX, ICON_DY});
    }
    if (customerOrder.hasPastry)
    {
        // Pastry icon (left) — frame 0 of props strip.
        createOrderIcon(assets,
                        0,
                        ICON_SIZE,
                        ICON_SIZE,
                        bubbleEnt,
                        {-ICON_DX, ICON_DY});
    }
}
bool cafe::MainGameScene::onUpdate(float dt)
{
    auto* renderer = getRenderer();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                return false;

            // TODO:move into input system
            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode == SDL_SCANCODE_SPACE
                    && !_machineEnt.get<CoffeeSpawner>().active)
                {
                    _machineEnt.get<CoffeeSpawner>().active = true;
                    std::cout << "[Pour] ON\n";
                }
                break;

            case SDL_EVENT_KEY_UP:
                if (event.key.scancode == SDL_SCANCODE_SPACE
                    && _machineEnt.get<CoffeeSpawner>().active)
                {
                    _machineEnt.get<CoffeeSpawner>().active = false;
                    std::cout << "[Pour] OFF\n";
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    const SDL_FPoint pos =
                        mouseWindowToRenderPoint(renderer,
                                                 event.button.x,
                                                 event.button.y);
                    dropSpacePickupSystem(pos);
                    dragStartSystem(pos);
                    enableSensorEventsOnHeldEntities();
                    _isDragging = true;
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (_isDragging)
                {
                    const SDL_FPoint pos =
                        mouseWindowToRenderPoint(renderer,
                                                 event.motion.x,
                                                 event.motion.y);
                    midDragSystem(pos);
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button == SDL_BUTTON_LEFT && _isDragging)
                {
                    // deliverySystem reads Held.dropSpaceEntity before
                    // dragReleaseSystem removes the Held component.
                    deliverySystem();
                    dragReleaseSystem();
                    _isDragging = false;
                }
                break;
            }
        }

        coffeeSpawnerSystem(dt, getAssetManager());    // spawn drops while pouring
        PhysicsContext::step(dt);
        liquidSensorEventSystem();        // count drops into cup; cleanup spilled
        dropSpaceDetectionSystem(); // update Held.dropSpaceEntity
        syncTransformFromBody();    // physics position -> Transform

        behaviorSystem(dt);         // tick patience; adds Leaving on timeout (fail)
        orderSystem();              // full cup + pastry -> rating=1 + Leaving (success)
        reportLeavingCustomers();   // log SUCCESSFUL / FAILED
        hierarchySystem();          // children follow parents; orphan children of Leaving
        customerCleanupSystem();            // destroy all Leaving entities

        SDL_RenderClear(renderer);
        drawSystem(renderer);       // sorted by renderLayer ascending
        SDL_RenderPresent(renderer);

    return true;
}
void cafe::MainGameScene::onCleanup()
{
    PhysicsContext::shutdown();
    std::cout << "[Main scene] Ended\n";
}