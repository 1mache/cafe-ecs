#include "Scene.h"

namespace cafe
{

void Scene::init(SDL_Renderer* renderer)
{
    _renderer = renderer;
    _assetManager.init(renderer);
}
} // namespace cafe
