#include "Assets.h"
#include "Utils.h"
#include <SDL3_image/SDL_image.h>

namespace cafe
{
// Loads a texture or terminates if the file is missing.
static SDL_Texture* loadOrDie(SDL_Renderer* r, const char* path, float& w, float& h)
{
    SDL_Texture* t = IMG_LoadTexture(r, path);
    assertFatal(t != nullptr, SDL_GetError());
    SDL_GetTextureSize(t, &w, &h);
    return t;
}

void Assets::init(SDL_Renderer* renderer)
{
    _particle = loadOrDie(renderer, "res/particle.png", _particleW, _particleH);
    _cup      = loadOrDie(renderer, "res/cup.png",      _cupW,      _cupH);
    _machine  = loadOrDie(renderer, "res/machine.png",  _machineW,  _machineH);
}

void Assets::shutdown()
{
    if (_particle) { SDL_DestroyTexture(_particle); _particle = nullptr; }
    if (_cup)      { SDL_DestroyTexture(_cup);      _cup      = nullptr; }
    if (_machine)  { SDL_DestroyTexture(_machine);  _machine  = nullptr; }
}

SDL_Texture* Assets::particle() { return _particle; }
SDL_Texture* Assets::cup()      { return _cup; }
SDL_Texture* Assets::machine()  { return _machine; }

SDL_FRect Assets::particleSrcRect() { return { 0.f, 0.f, _particleW, _particleH }; }
SDL_FRect Assets::cupSrcRect()      { return { 0.f, 0.f, _cupW,      _cupH }; }
SDL_FRect Assets::machineSrcRect()  { return { 0.f, 0.f, _machineW,  _machineH }; }

float Assets::particleW() { return _particleW; }
float Assets::particleH() { return _particleH; }
float Assets::cupW()      { return _cupW; }
float Assets::cupH()      { return _cupH; }
float Assets::machineW()  { return _machineW; }
float Assets::machineH()  { return _machineH; }
} // namespace cafe
