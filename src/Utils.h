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

/**
 * @brief Debug/release assertion. Calls calls fatalError() on failed conditions.
 * @param condition  Expression that must be true.
 * @param message    Shown if the condition fails.
 */
inline void assertFatal(bool condition, const char* message = "Assertion failed")
{
    if (!condition)
        fatalError(message);
}
} // namespace cafe