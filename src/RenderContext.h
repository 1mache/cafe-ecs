#pragma once
#include "GameConfig.h"
#include "Utils.h"
#include <SDL3/SDL.h>

namespace cafe
{
class RenderContext
{
public:
    static void init(SDL_Window* window, SDL_Renderer* renderer)
    {
        assertFatal(window != nullptr, "RenderContext::init: window is nullptr");
        assertFatal(renderer != nullptr, "RenderContext::init: renderer is nullptr");
        _window   = window;
        _renderer = renderer;
    };

    static SDL_Window* getWindow()
    {
        ensureState();
        return _window;
    }

    static SDL_Renderer* getRenderer()
    {
        ensureState();
        return _renderer;
    }

    static float getWinW()
    {
        ensureState();
        // if window will be resizable
        // int w;
        // SDL_GetWindowSize(_window, &w, nullptr);

        return START_WIN_W;
    }

    static float getWinH()
    {
        ensureState();
        // if window will be resizable
        // int h;
        // SDL_GetWindowSize(_window, nullptr, &h);

        return START_WIN_H;
    }

private:
    static void ensureState()
    {
        assertFatal(_window != nullptr, "RenderContext: window was not initialized");
        assertFatal(_renderer != nullptr,
                    "RenderContext: renderer was not initialized");
    }

    static inline SDL_Window*   _window{};
    static inline SDL_Renderer* _renderer{};
};
} // namespace cafe