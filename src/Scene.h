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
    SDL_Renderer* _renderer{};
    AssetManager  _assetManager;
    AudioContext  _audioContext;
    SceneId       _next{ SceneId::Quit };
};
} // namespace cafe
