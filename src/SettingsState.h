#pragma once

namespace cafe
{
/** @brief Process-global user settings. Survives scene swaps (the ECS world and
 *  AudioContext are torn down between scenes, so the mute flag cannot live in
 *  either). Modeled on DayState: a static service, no instances. Applied to
 *  audio in Scene::init, so every scene inherits the current setting. */
class SettingsState
{
public:
    static bool muted()       { return _muted; }
    static void toggleMuted() { _muted = !_muted; }

    static void resetAll()    { _muted = false; }

private:
    static inline bool _muted{ false };
};
} // namespace cafe
