#pragma once
#include "AssetManager.h"
#include "AudioContext.h"
#include "SceneId.h"

namespace cafe
{
class Scene
{
public:
    Scene() = default;

    // template method pattern
    void    init(SDL_Renderer* renderer);
    SceneId run();                       // returns the scene to switch to next
    void    cleanup();

    AssetManager& getAssetManager() { return _assetManager; }
    AudioContext& getAudioContext() { return _audioContext; }
    SDL_Renderer* getRenderer() const { return _renderer; }

    virtual ~Scene() = default;
    Scene(const Scene&)            = delete;
    Scene& operator=(const Scene&) = delete;
protected:
    virtual void onInit()           = 0;
    virtual bool onUpdate(float dt) = 0;   // return false to end this scene
    virtual void onCleanup()        = 0;

    /** Call before returning false from onUpdate to choose the next scene.
     *  A scene that ends without calling this quits the game. */
    void requestNext(SceneId id) { _next = id; }
private:
    // Shared frame-pacing logic used by the main loop and the fade loops.
    void limitFrameRate(Uint64 frameStart, Uint64 ticksPerFrame,
                         Uint64 performanceFrequency) const;

    // Grabs the currently presented (logical-resolution) frame as a texture.
    // Caller owns the returned texture and must SDL_DestroyTexture it.
    SDL_Texture* captureFrame() const;

    // Presents `frame` for FADE_DURATION seconds with a black overlay fading
    // from clear->opaque on top of it.
    void playFadeOut(SDL_Texture* frame) const;

    SDL_Renderer* _renderer{};
    AssetManager  _assetManager;
    AudioContext  _audioContext;
    SceneId       _next{ SceneId::Quit };
};
} // namespace cafe
