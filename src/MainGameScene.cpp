#include "MainGameScene.h"
#include "Components.h"
#include "Entities.h"
#include "PhysicsContext.h"
#include "Systems.h"

#include <iostream>

void cafe::MainGameScene::onInit()
{
    _physics.init();

    auto& assets = getAssetManager();

    createBg(assets, BG_PATH);
    createBartop(assets, _physics);

    createCoffeeMachine(assets, _physics, {-7.f, -1.f});

    // Supply is summoned on demand: click a button to drop a fresh cup/pastry in.
    createSpawnButton(assets, _physics, supply::CUP_BUTTON, DropType::Cup);
    createSpawnButton(assets, _physics, supply::PASTRY_BUTTON, DropType::Pastry);

    // Ice machine (gray placeholder square) with its spawn button on the machine face.
    createIceMachine(assets, supply::ICE_MACHINE_POS);
    createSpawnButton(assets, _physics, supply::ICE_BUTTON, DropType::Ice);

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
    supplyButtonSystem(getAssetManager(), _physics);

    // deliverySystem reads DragIntent.dropSpaceEntity on release before
    // dragAndDropSystem resets the intent to None.
    deliverySystem();
    checkBeverageSystem();        // snapshot cup contents -> CoffeeOverview
    acceptGradedBeverageSystem(); // grades cups with CheckCoffeeIntent + CoffeeOverview
    dragAndDropSystem();          // held: follow mouse; released: snap/drop

    machineButtonSystem();             // coffee-machine buttons -> pour state
    liquidSpawnerSystem(getAssetManager(), _physics, dt);    // spawn drops while pouring
    _physics.step(dt);
    liquidSensorEventSystem(_physics);  // count drops into cup; cleanup spilled
    dropSpaceDetectionSystem(_physics); // update DragIntent.dropSpaceEntity

    physicsToTransformSystem();    // physics position -> Transform

    customerSpawnerSystem(getAssetManager(), _physics, dt); // keep one customer at the seat
    behaviorSystem(dt);           // tick patience; adds Leaving on timeout
    orderSystem();                // all items served -> add Leaving (success)
    finalizeOrderGradeSystem();   // sum per-item grades + apply patience penalty -> Behavior.rating
    reportLeavingCustomers();     // log SUCCESSFUL / FAILED with final rating
    hierarchySystem();            // children follow parents; orphan children of Leaving
    customerCleanupSystem();      // destroy all Leaving entities

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