#pragma once
#include <SDL3/SDL.h>

namespace cafe
{
void eventGatheringSystem(SDL_Renderer* renderer);

inline SDL_FPoint mouseWindowToRenderPoint(SDL_Renderer* renderer,
                                           float         windowX,
                                           float         windowY)
{
    SDL_FPoint p{ windowX, windowY };
    SDL_RenderCoordinatesFromWindow(renderer,
                                    windowX, windowY, &p.x, &p.y);
    return p;
}
}