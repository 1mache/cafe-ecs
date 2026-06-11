#include "Scene.h"

namespace cafe
{

void Scene::init(SDL_Renderer* renderer)
{
    _renderer = renderer;
    _assetManager.init(renderer);

    onInit();
}
void Scene::run()
{
    // can add things here
    onRun();
    // and here!
}
void Scene::cleanup()
{
    onCleanup();
    _assetManager.clear();
}
} // namespace cafe
