#pragma once

namespace cafe::sound
{
// WAV file names under res/sounds/, passed to AudioContext::play / startSustained / playMusic.
    constexpr auto WATER_DROP      = "water_drop.wav";
    constexpr auto COFFEE          = "coffee.wav";
    constexpr auto MILK_STEAMER    = "milk_steamer.wav";
    constexpr auto BUTTON_PRESS    = "button_press.wav";
    constexpr auto BUY_UPGRADE     = "buy_upgrade.wav";
    constexpr auto GRAB            = "grab.wav";
    constexpr auto PUT_IN_MICROWAVE = "put_in_microwave.wav";
    constexpr auto MICROWAVE       = "microwave.wav";
    constexpr auto MICROWAVE_END   = "microwave_end.wav";
    constexpr auto PAPER           = "paper.wav";
    constexpr auto INTRO_MUSIC     = "intro_music.wav";   // declared; wired later
    constexpr auto MAIN_MUSIC      = "main_music.wav";
    constexpr auto MAIN_MUSIC_2    = "main_music2.wav";

    // Per-sound volume knobs (tune here; 1.0 = unchanged).
    constexpr float MUSIC_VOLUME     = 0.15f;   // background sits low
    constexpr float POUR_VOLUME      = 1.0f;
    constexpr float BUTTON_VOLUME    = 0.8f;
    constexpr float GRAB_VOLUME      = 0.9f;
    constexpr float UPGRADE_VOLUME   = 0.4f;
    constexpr float MICROWAVE_VOLUME = 1.0f;
    constexpr float PAPER_VOLUME     = 0.8f;
} // namespace cafe::sound