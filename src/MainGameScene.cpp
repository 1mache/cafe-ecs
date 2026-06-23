#include "MainGameScene.h"
#include "Components.h"
#include "DayState.h"
#include "Entities.h"
#include "Menu.h"
#include "PhysicsContext.h"
#include "Systems.h"
#include "Texture.h"

#include <iostream>

void cafe::MainGameScene::onInit()
{
    _physics.init();

    auto& assets = getAssetManager();

    // Validate pastry/coffee sprite counts against the enums before any customer spawns.
    constexpr auto PROPS_TEX  = "props.png";
    constexpr auto PROPS_DATA = "props.json";
    validateOrderSprites(assets.getSpriteSheet(PROPS_TEX, PROPS_DATA));

    createBg(assets, BG_PATH);
    createBartop(assets, _physics);

    createCoffeeMachine(assets, _physics, {-7.f, -1.f});

    // Supply is summoned on demand: click a button to drop a fresh cup/pastry in.
    createSpawnButton(assets, _physics, supply::CUP_BUTTON, DropType::Cup);
    createSpawnButton(assets, _physics, supply::PASTRY_BUTTON, DropType::Pastry);

    // Ice machine (gray placeholder square) with its spawn button on the machine face.
    createIceMachine(assets, supply::ICE_MACHINE_POS);
    createSpawnButton(assets, _physics, supply::ICE_BUTTON, DropType::Ice);

    // Microwave (gray placeholder square, no button): drag a pastry onto it to heat it.
    createMicrowave(assets, _physics, supply::MICROWAVE_POS);

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

    // --- Day cycle ---
    DayState::beginNewDay();

    // Day clock + its HUD anchor Transform (the day-progress bar reads this).
    constexpr float DAY_BAR_HALF_W = 8.0f; // world half-width => 16-unit span
    constexpr float DAY_BAR_Y      = 4.6f; // near the top of the canvas
    auto dayEntity = bagel::Entity::create();
    dayEntity.addAll(
        Transform{ .x = 0.f, .y = DAY_BAR_Y, .w = DAY_BAR_HALF_W, .h = 0.1f },
        DayClock{ .timeRemaining = DAY_LENGTH, .dayLength = DAY_LENGTH });

    // Day-progress bar: the SAME TimerBar component the microwave bar uses.
    // timerBarSystem() sizes it each frame from the DayClock fraction.
    const Texture& barTex = getAssetManager().getTexture("particle.png");
    auto dayBar = bagel::Entity::create();
    dayBar.addAll(
        Transform{ .x = 0.f, .y = DAY_BAR_Y, .w = 0.f, .h = 0.35f },
        Drawable{ barTex.get(), barTex.getFullSrcRect(), layer::UI1,
                  SDL_Color{ 90, 200, 120, 255 } },
        TimerBar{ .source = dayEntity });
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
    // Intake/cook a pastry. Must be after deliverySystem (which reads the same
    // released DragIntent) and before dragAndDropSystem (which would snap the pat).
    microwaveSystem(getAssetManager(), _physics, dt);
    checkBeverageSystem();        // snapshot cup contents -> CoffeeOverview
    acceptGradedBeverageSystem(); // grades cups with CheckCoffeeIntent + CoffeeOverview
    checkPastrySystem();          // grades pastries with CheckPastryIntent
    dragAndDropSystem();          // held: follow mouse; released: snap/drop

    machineButtonSystem();             // coffee-machine buttons -> pour state
    liquidSpawnerSystem(getAssetManager(), _physics, dt);    // spawn drops while pouring
    _physics.step(dt);
    liquidVelocityClampSystem();
    liquidSensorEventSystem(_physics);  // count drops into cup; cleanup spilled
    dropSpaceDetectionSystem(_physics); // update DragIntent.dropSpaceEntity

    physicsToTransformSystem();    // physics position -> Transform

    customerSpawnerSystem(getAssetManager(), _physics, dt); // keep one customer at the seat
    behaviorSystem(dt);           // tick patience; adds Leaving on timeout
    orderSystem();                // all items served -> add Leaving (success)
    finalizeOrderGradeSystem();   // sum per-item grades + apply patience penalty -> Behavior.rating
    recordDayResultsSystem();     // capture rating/succeeded into DayState before cleanup
    reportLeavingCustomers();     // log SUCCESSFUL / FAILED with final rating
    hierarchySystem();            // children follow parents; orphan children of Leaving
    customerCleanupSystem();      // destroy all Leaving entities
    cupAlphaSystem();             // fade cup front when contents > 0
    timerBarSystem();             // size microwave + day bars from their sources

    SDL_RenderClear(renderer);
    drawSystem(renderer);       // sorted by renderLayer ascending
    debugHighlightPhysics(renderer);
    SDL_RenderPresent(renderer);

    if (dayClockSystem(dt))
    {
        requestNext(SceneId::DayReport);
        return false;
    }
    return true;
}
void cafe::MainGameScene::onCleanup()
{
    destroyAllGameEntities();   // global ECS registry: clear it before the next scene
    _physics.cleanup();
    std::cout << "[Main scene] Ended\n";
}