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
        _audioContext.updateMusic(); // keep background music looping seamlessly

        limitFrameRate(frameStart, ticksPerFrame, performanceFrequency);
    }

    // ---- Fade out: grab the last presented frame and fade it to black. ----
    if (SDL_Texture* lastFrame = captureFrame())
    {
        playFadeOut(lastFrame);
        SDL_DestroyTexture(lastFrame);
    }

    return _next;
}

void Scene::limitFrameRate(Uint64 frameStart, Uint64 ticksPerFrame,
                            Uint64 performanceFrequency) const
{
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
    }
}

SDL_Texture* Scene::captureFrame() const
{
    SDL_Surface* surface = SDL_RenderReadPixels(_renderer, nullptr);
    if (!surface) return nullptr;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(_renderer, surface);
    SDL_DestroySurface(surface);
    return texture;
}

void Scene::playFadeOut(SDL_Texture* frame) const
{
    const Uint64 performanceFrequency = SDL_GetPerformanceFrequency();
    const Uint64 ticksPerFrame        = performanceFrequency / FPS;
    const Uint64 fadeStart            = SDL_GetPerformanceCounter();

    float elapsed = 0.f;
    while (elapsed < FADE_DURATION)
    {
        Uint64 frameStart = SDL_GetPerformanceCounter();
        elapsed = static_cast<float>(frameStart - fadeStart)
                / static_cast<float>(performanceFrequency);

        float progress = elapsed / FADE_DURATION;
        if (progress > 1.f) progress = 1.f;
        Uint8 alpha = static_cast<Uint8>(255.f * progress);

        SDL_RenderClear(_renderer);
        SDL_RenderTexture(_renderer, frame, nullptr, nullptr);
        SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(_renderer, 0, 0, 0, alpha);
        SDL_RenderFillRect(_renderer, nullptr);
        SDL_RenderPresent(_renderer);

        limitFrameRate(frameStart, ticksPerFrame, performanceFrequency);
    }
}
void Scene::cleanup()
{
    onCleanup();
    _assetManager.clear();
    _audioContext.cleanup();
}
} // namespace cafe
