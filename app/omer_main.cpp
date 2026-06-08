#include "Assets.h"
#include "Components.h"
#include "Entities.h"
#include "GameConfig.h"
#include "PhysicsContext.h"
#include "RenderContext.h"
#include "Systems.h"
#include "Utils.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

int main()
{
    using namespace cafe;

    {
        std::ifstream  infoFile("res/info.json");
        nlohmann::json info;
        infoFile >> info;
        std::cout << "message: "  << info["message"].get<std::string>()  << "\n"
                  << "location: " << info["location"].get<std::string>() << std::endl;
    }

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Init error : " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Window*   window{};
    SDL_Renderer* renderer{};
    SDL_CreateWindowAndRenderer("Cafe",
                                static_cast<int>(START_WIN_W),
                                static_cast<int>(START_WIN_H),
                                SDL_WINDOW_OPENGL,
                                &window,
                                &renderer);
    assertFatal(window != nullptr, SDL_GetError());
    assertFatal(renderer != nullptr, SDL_GetError());

    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    RenderContext::init(window, renderer);
    Assets::init(renderer);
    PhysicsContext::init();

    // Nearest-neighbor scaling on every texture (we want crisp pixel art).
    SDL_SetTextureScaleMode(Assets::particle(), SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(Assets::cup(),      SDL_SCALEMODE_NEAREST);
    SDL_SetTextureScaleMode(Assets::machine(),  SDL_SCALEMODE_NEAREST);

    // Static scenery — bg and counter are loaded directly for now.
    SDL_Texture* bgTex = IMG_LoadTexture(renderer, "res/bg.png");
    assertFatal(bgTex != nullptr, SDL_GetError());
    SDL_SetTextureScaleMode(bgTex, SDL_SCALEMODE_NEAREST);
    float bgW{}, bgH{};
    SDL_GetTextureSize(bgTex, &bgW, &bgH);

    SDL_Texture* counterTex = IMG_LoadTexture(renderer, "res/counter.png");
    assertFatal(counterTex != nullptr, SDL_GetError());
    SDL_SetTextureScaleMode(counterTex, SDL_SCALEMODE_NEAREST);
    float counterW{}, counterH{};
    SDL_GetTextureSize(counterTex, &counterW, &counterH);

    auto bgEnt = bagel::Entity::create();
    bgEnt.addAll(
        Drawable{ bgTex, {0.f, 0.f, bgW, bgH} },
        Transform{ .x=0.f, .y=0.f, .w=LOGICAL_W/(2.f*PTM), .h=LOGICAL_H/(2.f*PTM) });

    auto counterEnt = bagel::Entity::create();
    counterEnt.addAll(
        Drawable{ counterTex, {0.f, 0.f, counterW, counterH} },
        Transform{ .x=0.f,
                   .y=counterH/(2*PTM) - LOGICAL_H/(2.f*PTM),
                   .w=counterW/(2*PTM), .h=counterH/(2*PTM) });

    // Coffee machine + cup, both aligned on x = -4 so drops actually land in the cup.
    // Cleanup zone is the off-screen sensor that destroys spilled drops.
    auto machineEnt = createCoffeeMachine({ -4.f, 1.f }, { 0.f, -0.5f },
                                          Assets::machine(), Assets::machineW(), Assets::machineH());
    auto cupEnt     = createCup({ -4.f, -1.f },
                                Assets::cup(), Assets::cupW(), Assets::cupH(), 50);
    (void)            createCleanupZone();

    bool   isRunning = true;
    Uint64 lastTicks = SDL_GetTicks();
    while (isRunning)
    {
        const auto frameStart = SDL_GetTicks();
        // Real elapsed time so physics catches up if a frame stalls. Clamp at 50 ms
        // because PhysicsContext::step's accumulator clamps too — defense in depth.
        float dt = static_cast<float>(frameStart - lastTicks) * 0.001f;
        if (dt > 0.05f) dt = 0.05f;
        lastTicks = frameStart;

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                isRunning = false;
                break;

            // SPACE pours coffee. TODO(team): replace with a real game-rule trigger.
            // The !active / active guards prevent duplicate prints on key repeat.
            case SDL_EVENT_KEY_DOWN:
                if (event.key.scancode == SDL_SCANCODE_SPACE
                    && !machineEnt.get<CoffeeSpawner>().active)
                {
                    machineEnt.get<CoffeeSpawner>().active = true;
                    std::cout << "[Pour] ON" << std::endl;
                }
                break;
            case SDL_EVENT_KEY_UP:
                if (event.key.scancode == SDL_SCANCODE_SPACE
                    && machineEnt.get<CoffeeSpawner>().active)
                {
                    machineEnt.get<CoffeeSpawner>().active = false;
                    std::cout << "[Pour] OFF" << std::endl;
                }
                break;
            }
        }

        // Spawn -> step physics -> consume sensor events -> sync transforms -> draw.
        // Order matters: spawn before step so new drops participate this frame,
        // sensor events come after the step, transforms last so we don't sync dead entities.
        coffeeSpawnerSystem(dt, Assets::particle());
        PhysicsContext::step(dt);
        sensorEventSystem();
        syncTransformFromBody();
        dumpDebugStatsEvery(dt);

        SDL_RenderClear(renderer);
        drawSystem();
        SDL_RenderPresent(renderer);

        // Frame pacing — cap at FPS. SDL_GetTicks is Uint64, SDL_Delay takes Uint32.
        constexpr Uint64 frameDeltaT = static_cast<Uint64>(FRAME_DELTA_MS);
        auto             frameEnd    = SDL_GetTicks();
        if (frameEnd - frameStart < frameDeltaT)
            SDL_Delay(static_cast<Uint32>(frameDeltaT - (frameEnd - frameStart)));
    }

    // Teardown order: physics first (its bodies don't reference SDL), then assets,
    // then directly-loaded textures, then renderer + window, then SDL.
    PhysicsContext::shutdown();
    Assets::shutdown();
    SDL_DestroyTexture(bgTex);
    SDL_DestroyTexture(counterTex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}
