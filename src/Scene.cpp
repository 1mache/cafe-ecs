#include "Scene.h"

#include "GameConfig.h"
#include "SettingsState.h"

namespace cafe
{

void Scene::init(SDL_Renderer* renderer)
{
    _renderer = renderer;
    _assetManager.init(renderer);
    _audioContext.init();

    // Each scene owns a fresh AudioContext, so the persistent mute flag must be
    // re-applied on every scene start.
    _audioContext.setVolume(SettingsState::muted() ? 0.f : 1.f);

    onInit();
}
SceneId Scene::run()
{
    const Uint64 performanceFrequency = SDL_GetPerformanceFrequency();
    const Uint64 ticksPerFrame        = performanceFrequency / FPS;

    Uint64 lastFrameStart = SDL_GetPerformanceCounter();
    bool   isRunning      = true;

    while (isRunning)
    {
        // ============= FRAME EQUALIZER + dt ============
        Uint64 frameStart = SDL_GetPerformanceCounter();
        float  dt         = static_cast<float>(frameStart - lastFrameStart)
                          / static_cast<float>(performanceFrequency);
        if (dt > 0.05f) dt = 0.05f;
        lastFrameStart = frameStart;
        // ============= FRAME EQUALIZER + dt ============

        // actual game stuff
        isRunning = onUpdate(dt);

        // ============= FRAME EQUALIZER ============

        Uint64 frameEnd     = SDL_GetPerformanceCounter();
        Uint64 frameElapsed = frameEnd - frameStart;

        if (frameElapsed < ticksPerFrame)
        {
            Uint64 ticksToWait = ticksPerFrame - frameElapsed;

            // 1. Coarse Sleep: Convert to milliseconds and sleep for the bulk of the time.
            // We subtract a 2ms buffer because SDL_Delay frequently oversleeps.
            Uint32 msToWait = static_cast<Uint32>((ticksToWait * 1000) / performanceFrequency);
            if (msToWait > 2)
                SDL_Delay(msToWait - 2);

            // 2. Fine Tuning: Spin-lock for the remaining micro-seconds to hit the exact target.
            while (SDL_GetPerformanceCounter() - frameStart < ticksPerFrame)
            {
                // High-precision wait loop
            }
        // ============= FRAME EQUALIZER ============
        }
    }
    return _next;
}
void Scene::cleanup()
{
    onCleanup();
    _assetManager.clear();
    _audioContext.cleanup();
}
} // namespace cafe
