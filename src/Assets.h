#pragma once
#include <SDL3/SDL.h>

namespace cafe
{
/** @brief Loads and caches the textures shared across factories. */
class Assets
{
public:
    static void init(SDL_Renderer* renderer);
    static void shutdown();

    static SDL_Texture* particle();
    static SDL_Texture* cup();
    static SDL_Texture* machine();

    static SDL_FRect particleSrcRect();
    static SDL_FRect cupSrcRect();
    static SDL_FRect machineSrcRect();

    static float particleW();
    static float particleH();
    static float cupW();
    static float cupH();
    static float machineW();
    static float machineH();

private:
    static inline SDL_Texture* _particle{};
    static inline SDL_Texture* _cup{};
    static inline SDL_Texture* _machine{};
    static inline float        _particleW{}, _particleH{};
    static inline float        _cupW{},      _cupH{};
    static inline float        _machineW{},  _machineH{};
};
} // namespace cafe
