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
    std::cout << "[Order]";
    if (customerOrder.hasDrink)
        std::cout << " drink=" << temperatureName(customerOrder.drinkTemperature)
                  << ' ' << recipe.name;
    if (customerOrder.hasPastry)
        std::cout << " pastry=" << temperatureName(customerOrder.pastryTemperature)
                  << ' ' << pastryName(customerOrder.pastry);
    std::cout << '\n';
    auto customerEnt = createCustomer(assets, { 5.f, -1.f }, customerOrder, 60.f);

    // --- Speech bubble + order icons (children of customer) ---
    auto bubbleEnt = createSpeechBubble(
        assets, customerEnt,
        {0.f, 28.f});
    // --- Order icons (children of the bubble) ---
    // The bubble is split into 4 equal columns (quarters of its width):
    //   0: beverage   1: beverage temp (blank)   2: pastry   3: pastry temp (blank)
    // Each column can stack up to BUBBLE_MAX_ICONS_PER_COLUMN icons vertically
    // (stacking logic not implemented yet — icons are sized so they would fit).
    constexpr float BUBBLE_W  = BUBBLE_DIMS.x * BUBBLE_SCALE; // on-screen px
    constexpr float BUBBLE_H  = BUBBLE_DIMS.y * BUBBLE_SCALE; // on-screen px
    constexpr float COL_W     = BUBBLE_W / 4.f;
    constexpr float ROW_H     = BUBBLE_H / BUBBLE_MAX_ICONS_PER_COLUMN;
    // Icon must fit both a quarter-width column and one stacked row.
    constexpr float ICON_SIZE = COL_W < ROW_H ? COL_W : ROW_H;
    constexpr float ICON_Y    = 0.f;                          // bubble center
    // Column centers, left -> right, relative to bubble center (+x = right).
    constexpr float COL_X[4]  = {-1.5f * COL_W, -0.5f * COL_W,
                                 +0.5f * COL_W, +1.5f * COL_W};

    if (customerOrder.hasDrink)
    {
        // Column 0: beverage icon — this drink's coffee frame in props.png.
        createOrderIcon(assets, recipe.iconFrame, ICON_SIZE, ICON_SIZE,
                        bubbleEnt, {COL_X[0], ICON_Y});
    }
    // Column 1: beverage temperature — no art yet, intentionally blank.

    if (customerOrder.hasPastry)
    {
        // Column 2: pastry icon — this order's pastry frame in props.png.
        createOrderIcon(assets, static_cast<int>(customerOrder.pastry),
                        ICON_SIZE, ICON_SIZE,
                        bubbleEnt, {COL_X[2], ICON_Y});
    }
    // Column 3: pastry temperature — no art yet, intentionally blank.
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