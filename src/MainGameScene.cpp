#include "MainGameScene.h"
#include "Components.h"
#include "Entities.h"
#include "Menu.h"
#include "PhysicsContext.h"
#include "SpriteDims.h"
#include "Systems.h"

#include <iostream>

void cafe::MainGameScene::onInit()
{
    PhysicsContext::init();

    auto& assets = getAssetManager();

    // Single input-state entity: intentSystem polls SDL and publishes here.
    _inputEnt = bagel::Entity::create();
    _inputEnt.add(SdlEvents{});

    createBg(assets, BG_PATH);
    createBartop(assets);

    auto machine = createCoffeeMachine(assets, {-6.f, -1.f});
    for (size_t i = 0; i < INGREDIENT_COUNT; ++i)
        _pipes[i] = machine.pipes[i];

    createCup(assets, CUP_SLOT, CUP_CAPACITY);
    createPastry(PASTRY_SLOT, assets);

    // Cleanup zone: off-screen sensor destroys spilled drops.
    createCleanupZone();

    std::cout << "[Demo] Hold 1/2/3 to pour Coffee/Milk/Water. Left-drag to move the cup or pastry.\n"
              << "[Demo] Serve a cup matching the order ratio + a pastry to the customer in 60 s.\n";

    // --- Customer cycle ---
    // One spawner entity drives the loop: it keeps a single customer at the seat,
    // spawning the next (with a fresh random order) SPAWN_INTERVAL s after it leaves.
    // cooldown = 0 so the first customer appears on the first frame.
    auto spawner = bagel::Entity::create();
    spawner.add(Spawner{ .seat     = { 5.f, -1.f },
                         .patience = CUSTOMER_PATIENCE,
                         .interval = SPAWN_INTERVAL,
                         .cooldown = 0.f });
}
bool cafe::MainGameScene::onUpdate(float dt)
{
    auto* renderer = getRenderer();

        // Single SDL poll: publishes SdlEvents and sets DragIntent transitions.
        intentSystem(renderer);

        if (isTriggeredEvent(_inputEnt.entity(), Controls::Quit))
            return false;

        // deliverySystem reads DragIntent.dropSpaceEntity on release before
        // dragAndDropSystem resets the intent to None.
        deliverySystem();
        dragAndDropSystem();        // held: follow mouse; released: snap/drop

        liquidSpawnerSystem(dt, getAssetManager());    // spawn drops while pouring
        PhysicsContext::step(dt);
        liquidSensorEventSystem();        // count drops into cup; cleanup spilled
        dropSpaceDetectionSystem(); // update DragIntent.dropSpaceEntity

        syncTransformFromBody();    // physics position -> Transform

        customerSpawnerSystem(dt, getAssetManager()); // keep one customer at the seat
        behaviorSystem(dt);         // tick patience; adds Leaving on timeout (fail)
        orderSystem();              // full cup + pastry -> rating=1 + Leaving (success)
        reportLeavingCustomers();   // log SUCCESSFUL / FAILED
        hierarchySystem();          // children follow parents; orphan children of Leaving
        recycleDeliveredItems();    // order done/abandoned -> send cup+pastry home, purge drops
        customerCleanupSystem();            // destroy all Leaving entities

        SDL_RenderClear(renderer);
        drawSystem(renderer);       // sorted by renderLayer ascending
        debugDrawCupWalls(renderer);
        SDL_RenderPresent(renderer);

    return true;
}
void cafe::MainGameScene::onCleanup()
{
    PhysicsContext::shutdown();
    std::cout << "[Main scene] Ended\n";
}