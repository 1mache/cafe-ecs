#pragma once
#include <SDL3/SDL.h>
#include <random>
#include <string_view>
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
inline void assertFatal(bool condition, std::string_view message = "Assertion failed")
{
    if (!condition)
        fatalError(message.data());
}
inline std::mt19937& getRng()
{
    static std::mt19937 rng{ std::random_device{}() };
    return rng;
}
} // namespace cafe