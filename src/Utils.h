#pragma once
#include <SDL3/SDL.h>
#include <cassert>
#include <cstdlib>

namespace cafe
{
/** @brief Logs @p message, shows a message box, and terminates immediately. */
[[noreturn]] inline void fatalError(const char* message)
{
    SDL_Log("Fatal error: %s", message);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", message, nullptr);
    SDL_Quit();
    std::exit(1);
}
}