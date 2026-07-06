#pragma once
#include <SDL3/SDL.h>
#include <random>
#include <string_view>
#include <cstdlib>

namespace cafe
{
template <typename T>
requires std::integral<T>
constexpr std::size_t toSizet(T value)
{
    return static_cast<size_t>(value);
}

/** @brief Logs @p message, shows a message box, and terminates immediately. */
[[noreturn]] inline void fatalError(std::string_view message)
{
#ifdef DEBUG
    SDL_Log("Fatal error: %s", message.data());
#endif
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", message.data(), nullptr);
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