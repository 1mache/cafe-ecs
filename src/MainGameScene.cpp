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

    createCup(assets, {-4.f, -1.f}, CUP_CAPACITY);
    createPastry({4.f, -3.f},  assets);

    // Cleanup zone: off-screen sensor destroys spilled drops.
    createCleanupZone();

    std::cout << "[Demo] Hold 1/2/3 to pour Coffee/Milk/Water. Left-drag to move the cup or pastry.\n"
              << "[Demo] Serve a cup matching the order ratio + a pastry to the customer in 60 s.\n";

    // --- Customer ---
    Order customerOrder = randomDrinkOrder(/*hasPastry=*/ true);
    const DrinkRecipe& recipe = recipeFor(customerOrder.drink);
    std::cout << "[Order] " << temperatureName(customerOrder.temperature)
              << ' ' << recipe.name << '\n';
    auto customerEnt = createCustomer(assets, { 5.f, -1.f }, customerOrder, 60.f);

    // --- Speech bubble + order icons (children of customer) ---
    auto bubbleEnt = createSpeechBubble(
        assets, customerEnt,
        {0.f, 24.f});
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

        behaviorSystem(dt);         // tick patience; adds Leaving on timeout (fail)
        orderSystem();              // full cup + pastry -> rating=1 + Leaving (success)
        reportLeavingCustomers();   // log SUCCESSFUL / FAILED
        hierarchySystem();          // children follow parents; orphan children of Leaving
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