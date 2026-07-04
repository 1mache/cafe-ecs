#include "MainGameScene.h"

#include "AnimationSystem.h"
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
    createNapkin(assets, _physics);

    // Ice machine.
    createIceMachine(assets, _physics, supply::ICE_MACHINE_POS);
    // Then coffee machine on top (in front). order matters
    createCoffeeMachine(assets, _physics, supply::COFFEE_MACHINE_POS);

    // Supply is summoned on demand: click a button to drop a fresh cup in.
    // (The pastry button is the TV icon, created inside createPastryTv below.)
    createSpawnButton(assets, _physics, supply::CUP_BUTTON_POS, DropType::Cup);


    // Microwave (gray placeholder square, no button): drag a pastry onto it to heat it.
    createMicrowave(assets, _physics, supply::MICROWAVE_POS);

    // Pastry TV: top-left display cycling pastry types; the pastry button spawns
    // whichever pastry it currently shows.
    createPastryTv(assets, _physics, supply::PASTRY_TV_POS);

    // Cleanup zone: off-screen sensor destroys spilled drops.
    createCleanupZone(_physics);

    // --- Customer cycle ---
    // One spawner entity drives the loop: it keeps a single customer at the seat,
    // spawning the next (with a fresh random order) SPAWN_INTERVAL s after it leaves.
    // cooldown = 0 so the first customer appears on the first frame. The customer's
    // speech bubble + order-icon grid are built per-spawn in spawnCustomer().
    auto spawner = bagel::Entity::create();
    spawner.add(CustomerSpawner{ .seat     = { 5.9f, 0.f },
                         .interval = SPAWN_INTERVAL,
                         .cooldown = 0.f });

    // --- Day cycle ---
    DayState::beginNewDay();

    // Day-progress driver: the day ends after CUSTOMERS_PER_DAY customers (served+lost).
    auto dayEntity = bagel::Entity::create();
    dayEntity.add(DayProgress{ .target = CUSTOMERS_PER_DAY });

    // Apply purchased upgrades to this day's machines (level 0 = base values).
    applyUpgradesSystem();
}
bool cafe::MainGameScene::onUpdate(float dt)
{
    auto* renderer = getRenderer();

    bool exitRequested = false;
    // Single SDL poll: turns user input into per-entity intents.
    intentSystem(renderer, exitRequested);
    if (exitRequested) return false;

    // Rotate the pastry TV before reading it: keeps the shown pastry current.
    pastryTvSystem(getAssetManager(), dt);
    // Click a supply button to drop a fresh cup/ice into a free slot.
    supplyButtonSystem(getAssetManager(), _physics);
    // Pastry has its own system: its button is the TV icon, spawns the shown type.
    pastrySupplySystem(getAssetManager(), _physics);

    // deliverySystem reads DragIntent.dropSpaceEntity on release before
    // dragAndDropSystem resets the intent to None.
    deliverySystem();
    // Intake/cook a pastry. Must be after deliverySystem (which reads the same
    // released DragIntent) and before dragAndDropSystem (which would snap the pat).
    microwaveSystem(getAssetManager(), _physics, dt);
    checkBeverageSystem();        // snapshot cup contents -> CoffeeOverview
    acceptGradedBeverageSystem(); // grades cups with CheckCoffeeIntent + CoffeeOverview
    checkPastrySystem();          // grades pastries with CheckPastryIntent
    orderCheckmarkSystem();       // reveal bubble checkmarks for served slots
    dragAndDropSystem();          // held: follow mouse; released: snap/drop
    napkinSystem(getAssetManager(), _physics, dt);   // napkin state -> body velocity + size ease

    machineButtonSystem();             // coffee-machine buttons -> pour state
    liquidSpawnerSystem(getAssetManager(), _physics, getAudioContext(), dt);    // spawn drops while pouring
    _physics.step(dt);
    liquidVelocityClampSystem(); // TODO: remove and check effects in the end
    liquidSensorEventSystem(_physics);  // count drops into cup; cleanup spilled
    dropSpaceDetectionSystem(_physics); // update DragIntent.dropSpaceEntity

    physicsToTransformSystem();    // physics position -> Transform

    customerSpawnerSystem(getAssetManager(), _physics, dt); // keep one customer at the seat
    animationSystem(getAssetManager(), dt);
    particleSystem(dt);           // drift + fade active FX particles
    lifetimeSystem(dt);           // reap expired FX entities
    behaviorSystem(dt);           // tick patience; adds Leaving on timeout
    orderSystem();                // all items served -> add Leaving (success)
    finalizeOrderGradeSystem();   // sum per-item grades + apply patience penalty -> Behavior.rating
    recordDayResultsSystem();     // capture rating/succeeded into DayState before cleanup
    gradePopupSystem();           // spawn graded text + particle burst at the customer
    reportLeavingCustomers();     // log SUCCESSFUL / FAILED with final rating
    positionHierarchySystem();            // children follow parents; orphan children of Leaving
    customerCleanupSystem();      // destroy all Leaving entities
    cupAlphaSystem();             // fade cup front when contents > 0

    SDL_RenderClear(renderer);
    drawSystem(renderer);       // sorted by renderLayer ascending
    drawTextSystem(renderer, getAssetManager().getTexture("font.png"));
    debugHighlightPhysics(renderer);
    SDL_RenderPresent(renderer);

    if (dayEndSystem())
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