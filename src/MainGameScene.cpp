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
    _physics.init();

    auto& assets = getAssetManager();

    createBg(assets, BG_PATH);
    createBartop(assets, _physics);

    createCoffeeMachine(_physics, assets, {-7.f, -1.f});

    // Supply is summoned on demand: click a button to drop a fresh cup/pastry in.
    createSpawnButton(assets, supply::CUP_BUTTON, DropType::cup);
    createSpawnButton(assets, supply::PASTRY_BUTTON, DropType::pastry);

    // Cleanup zone: off-screen sensor destroys spilled drops.
    createCleanupZone(_physics);

    std::cout << "[Demo] Hold 1/2/3 to pour Coffee/Water/Milk. Left-drag to move the cup or pastry.\n"
              << "[Demo] Serve a cup matching the order ratio + a pastry to the customer in 60 s.\n";

    // --- Customer cycle ---
    // One spawner entity drives the loop: it keeps a single customer at the seat,
    // spawning the next (with a fresh random order) SPAWN_INTERVAL s after it leaves.
    // cooldown = 0 so the first customer appears on the first frame. The customer's
    // speech bubble + order-icon grid are built per-spawn in spawnCustomer().
    auto spawner = bagel::Entity::create();
    spawner.add(Spawner{ .seat     = { 5.f, -1.f },
                         .patience = CUSTOMER_PATIENCE,
                         .interval = SPAWN_INTERVAL,
                         .cooldown = 0.f });
}
bool cafe::MainGameScene::onUpdate(float dt)
{
    auto* renderer = getRenderer();

    bool exitRequested = false;
    // Single SDL poll: turns user input into per-entity intents.
    intentSystem(renderer, exitRequested);
    if (exitRequested) return false;

    // Click a supply button to drop a fresh cup/pastry into a free slot.
    supplyButtonSystem(_physics, getAssetManager());

    // deliverySystem reads DragIntent.dropSpaceEntity on release before
    // dragAndDropSystem resets the intent to None.
    deliverySystem();
    dragAndDropSystem();        // held: follow mouse; released: snap/drop

    buttonSystem();             // coffee-machine buttons -> pour state
    liquidSpawnerSystem(_physics, dt, getAssetManager());    // spawn drops while pouring
    _physics.step(dt);
    liquidSensorEventSystem(_physics);  // count drops into cup; cleanup spilled
    dropSpaceDetectionSystem(_physics); // update DragIntent.dropSpaceEntity

    fallingSystem(dt);          // cartoonish drop-in; lands items into their slots
    syncTransformFromBody();    // physics position -> Transform

    customerSpawnerSystem(_physics, dt, getAssetManager()); // keep one customer at the seat
    behaviorSystem(dt);         // tick patience; adds Leaving on timeout (fail)
    orderSystem();              // full cup + pastry -> rating=1 + Leaving (success)
    reportLeavingCustomers();   // log SUCCESSFUL / FAILED
    hierarchySystem();          // children follow parents; orphan children of Leaving
    clearDeliveredItems();      // order done/abandoned -> destroy that customer's tray + drops
    customerCleanupSystem();    // destroy all Leaving entities

    SDL_RenderClear(renderer);
    drawSystem(renderer);       // sorted by renderLayer ascending
    debugHighlightPhysics(renderer);
    SDL_RenderPresent(renderer);

    return true;
}
void cafe::MainGameScene::onCleanup()
{
    _physics.cleanup();
    std::cout << "[Main scene] Ended\n";
}