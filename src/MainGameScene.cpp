#include "MainGameScene.h"
#include "Components.h"
#include "Entities.h"
#include "PhysicsContext.h"
#include "SpriteDims.h"
#include "Systems.h"

#include <iostream>

namespace
{
// TODO: move pour-key handling into the InputSystem/InputManager once it exists;
// the scene should read a PourIntent-writing input system, not poll SDL directly.
// Maps a key to a pour pipe index (Ingredient order). -1 if the key isn't a pour key.
int pipeIndexForScancode(SDL_Scancode sc)
{
    switch (sc)
    {
    case SDL_SCANCODE_1: return static_cast<int>(cafe::Ingredient::Coffee);
    case SDL_SCANCODE_2: return static_cast<int>(cafe::Ingredient::Milk);
    case SDL_SCANCODE_3: return static_cast<int>(cafe::Ingredient::Water);
    default:             return -1;
    }
}
} // namespace

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

        // Single SDL poll: publishes SdlEvents and sets DragIntent transitions.
        intentSystem(renderer);

        const auto& input = _inputEnt.get<SdlEvents>();

        if (isTriggeredEvent(_inputEnt.entity(), Controls::Quit))
            return false;

        // SPACE toggles coffee pour (key down/up edges).
        if (isTriggeredEvent(_inputEnt.entity(), Controls::KeyDown)
            && input.keyScancode == SDL_SCANCODE_SPACE
            && !_pipes[0].get<PourIntent>().active)
        {
            _pipes[0].get<PourIntent>().active = true;
            std::cout << "[Pour] ON\n";
        }
        if (isTriggeredEvent(_inputEnt.entity(), Controls::KeyUp)
            && input.keyScancode == SDL_SCANCODE_SPACE
            && _pipes[0].get<PourIntent>().active)
        {
            _pipes[0].get<PourIntent>().active = false;
            std::cout << "[Pour] OFF\n";
        }

        // deliverySystem reads DragIntent.dropSpaceEntity on release before
        // dragAndDropSystem resets the intent to None.
        deliverySystem();
        dragAndDropSystem();        // held: follow mouse; released: snap/drop

        pourControlSystem();                           // PourIntent -> CoffeeSpawner.active
        coffeeSpawnerSystem(dt, getAssetManager());    // spawn drops while pouring
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
        SDL_RenderPresent(renderer);

    return true;
}
void cafe::MainGameScene::onCleanup()
{
    PhysicsContext::shutdown();
    std::cout << "[Main scene] Ended\n";
}