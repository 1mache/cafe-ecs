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
    Order customerOrder = randomOrder();
    std::cout << "[Order] " << customerOrder.drinkCount << " drink(s), "
              << customerOrder.pastryCount << " pastry(ies):\n";
    for (int i = 0; i < customerOrder.drinkCount; ++i)
        std::cout << "  drink "  << (i + 1) << ": "
                  << temperatureName(customerOrder.drinks[i].temp) << ' '
                  << recipeFor(customerOrder.drinks[i].type).name << '\n';
    for (int i = 0; i < customerOrder.pastryCount; ++i)
        std::cout << "  pastry " << (i + 1) << ": "
                  << temperatureName(customerOrder.pastries[i].temp) << ' '
                  << pastryName(customerOrder.pastries[i].type) << '\n';
    auto customerEnt = createCustomer(assets, { 5.f, -1.f }, customerOrder, 60.f);

    // --- Speech bubble + order icons (children of customer) ---
    auto bubbleEnt = createSpeechBubble(
        assets, customerEnt,
        {0.f, 28.f});
    // --- Order icons (children of the bubble) ---
    // The bubble is split into 4 equal columns (quarters of its width):
    //   0: beverage   1: beverage temp   2: pastry   3: pastry temp
    // Items stack vertically within their column (row 0 top -> row 2 bottom),
    // up to BUBBLE_MAX_ICONS_PER_COLUMN per column.
    constexpr float BUBBLE_W  = BUBBLE_DIMS.x * BUBBLE_SCALE; // on-screen px
    constexpr float BUBBLE_H  = BUBBLE_DIMS.y * BUBBLE_SCALE; // on-screen px
    constexpr float COL_W     = BUBBLE_W / 4.f;
    constexpr float ROW_H     = BUBBLE_H / BUBBLE_MAX_ICONS_PER_COLUMN;
    // Icon must fit both a quarter-width column and one stacked row, then 0.7x.
    constexpr float ICON_SIZE = (COL_W < ROW_H ? COL_W : ROW_H) * 0.7f;
    // Pull icon centers toward the bubble center to tighten the gaps between them.
    constexpr float GAP       = 0.8f;
    constexpr float COL_DX    = COL_W * GAP;
    constexpr float ROW_DY    = ROW_H * GAP;
    // Column centers, left -> right, relative to bubble center (+x = right).
    constexpr float COL_X[4]  = {-1.5f * COL_DX, -0.5f * COL_DX,
                                 +0.5f * COL_DX, +1.5f * COL_DX};
    // Row centers, top -> bottom, relative to bubble center (+y = up).
    constexpr float ROW_Y[BUBBLE_MAX_ICONS_PER_COLUMN] = {+ROW_DY, 0.f, -ROW_DY};

    for (int i = 0; i < customerOrder.drinkCount; ++i)
    {
        const DrinkItem& d = customerOrder.drinks[i];
        // Column 0: beverage icon — this drink's coffee frame in props.png.
        createOrderIcon(assets, recipeFor(d.type).iconFrame, ICON_SIZE, ICON_SIZE,
                        bubbleEnt, {COL_X[0], ROW_Y[i]});
        // Column 1: beverage temperature — fire (Hot) or ice (Cold).
        createOrderIcon(assets, temperatureFrame(d.temp), ICON_SIZE, ICON_SIZE,
                        bubbleEnt, {COL_X[1], ROW_Y[i]});
    }

    for (int i = 0; i < customerOrder.pastryCount; ++i)
    {
        const PastryItem& p = customerOrder.pastries[i];
        // Column 2: pastry icon — this pastry's frame in props.png.
        createOrderIcon(assets, static_cast<int>(p.type), ICON_SIZE, ICON_SIZE,
                        bubbleEnt, {COL_X[2], ROW_Y[i]});
        // Column 3: pastry temperature — fire (Hot) or ice (Cold).
        createOrderIcon(assets, temperatureFrame(p.temp), ICON_SIZE, ICON_SIZE,
                        bubbleEnt, {COL_X[3], ROW_Y[i]});
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
        debugHighlightPhysics(renderer);
        SDL_RenderPresent(renderer);

    return true;
}
void cafe::MainGameScene::onCleanup()
{
    PhysicsContext::shutdown();
    std::cout << "[Main scene] Ended\n";
}