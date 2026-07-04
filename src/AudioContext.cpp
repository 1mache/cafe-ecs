#include "AudioContext.h"
#include "Utils.h"

namespace cafe
{
void AudioContext::init()
{
    assertFatal(_device == 0, "AudioContext::init called twice");

    _device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    assertFatal(_device != 0, std::string("SDL_OpenAudioDevice failed: ") + SDL_GetError());

    // Streams bound to one device are mixed together by SDL's audio thread.
    // A pre-built pool lets play() reuse an idle stream without allocating per sound.
    for (SDL_AudioStream*& voice : _voices)
    {
        voice = SDL_CreateAudioStream(nullptr, nullptr);
        assertFatal(voice != nullptr, std::string("SDL_CreateAudioStream failed: ") + SDL_GetError());
        SDL_BindAudioStream(_device, voice);
    }

    _sustainedVoice = SDL_CreateAudioStream(nullptr, nullptr);
    assertFatal(_sustainedVoice != nullptr, std::string("SDL_CreateAudioStream failed: ") + SDL_GetError());
    SDL_BindAudioStream(_device, _sustainedVoice);

    _musicVoice = SDL_CreateAudioStream(nullptr, nullptr);
    assertFatal(_musicVoice != nullptr, std::string("SDL_CreateAudioStream failed: ") + SDL_GetError());
    SDL_BindAudioStream(_device, _musicVoice);

    _microwaveVoice = SDL_CreateAudioStream(nullptr, nullptr);
    assertFatal(_microwaveVoice != nullptr, std::string("SDL_CreateAudioStream failed: ") + SDL_GetError());
    SDL_BindAudioStream(_device, _microwaveVoice);
}

void AudioContext::cleanup()
{
    for (SDL_AudioStream*& voice : _voices)
    {
        if (voice)
        {
            SDL_DestroyAudioStream(voice); // also unbinds from the device
            voice = nullptr;
        }
    }
    if (_sustainedVoice)
    {
        SDL_DestroyAudioStream(_sustainedVoice);
        _sustainedVoice = nullptr;
    }
    _sustainedName.clear();
    if (_musicVoice)
    {
        SDL_DestroyAudioStream(_musicVoice);
        _musicVoice = nullptr;
    }
    _musicName.clear();
    if (_microwaveVoice)
    {
        SDL_DestroyAudioStream(_microwaveVoice);
        _microwaveVoice = nullptr;
    }
    if (_device != 0)
    {
        SDL_CloseAudioDevice(_device);
        _device = 0;
    }
    _sounds.clear();
}

const Sound& AudioContext::getSound(std::string_view filename)
{
    std::string fullPath = std::string(RES_DIR_PATH) + std::string(filename);
    auto [it, success] = _sounds.try_emplace(filename.data());
    if (success) // key didnt previously exist
    {
        bool loaded = it->second.loadFromFile(fullPath);
        assertFatal(loaded, "Error loading " + fullPath + ": " + SDL_GetError());
    }

    return it->second;
}

void AudioContext::play(std::string_view filename, float volume)
{
    const Sound& sound = getSound(filename);

    // Reuse the first voice with nothing left to play. If all are busy, drop the
    // request (fire-and-forget). ponytail: 8 fixed voices, request dropped when full;
    // grow the pool or steal the quietest voice if simultaneity matters.
    for (SDL_AudioStream* voice : _voices)
    {
        if (SDL_GetAudioStreamQueued(voice) > 0)
            continue;

        SDL_SetAudioStreamFormat(voice, &sound.spec(), nullptr);
        SDL_SetAudioStreamGain(voice, volume);
        SDL_PutAudioStreamData(voice, sound.data(), static_cast<int>(sound.size()));
        return;
    }
}

void AudioContext::startSustained(std::string_view filename, float startOffsetSeconds, float volume)
{
    const Sound& sound = getSound(filename);

    // Skip startOffsetSeconds into the file. Offset is frame-aligned by construction
    // (whole frames * frame size), so playback stays sample-correct.
    const Uint32 frameSize = SDL_AUDIO_BYTESIZE(static_cast<Uint32>(sound.spec().format))
                            * static_cast<Uint32>(sound.spec().channels);
    Uint32 offset = static_cast<Uint32>(startOffsetSeconds * static_cast<float>(sound.spec().freq))
                  * frameSize;
    if (offset > sound.size()) offset = sound.size();
    const int chunk = static_cast<int>(sound.size() - offset);

    if (_sustainedName != filename)
    {
        SDL_ClearAudioStream(_sustainedVoice); // drop whatever was sustaining before
        SDL_SetAudioStreamFormat(_sustainedVoice, &sound.spec(), nullptr);
        SDL_SetAudioStreamGain(_sustainedVoice, volume);
        _sustainedName = filename;
    }

    // Loop: caller re-invokes every frame while holding. Queue another copy (from the
    // offset, so a fade-in is skipped on every repeat) whenever less than one full copy
    // remains buffered, keeping playback seamless.
    if (SDL_GetAudioStreamQueued(_sustainedVoice) < chunk)
        SDL_PutAudioStreamData(_sustainedVoice, sound.data() + offset, chunk);
}

void AudioContext::stopSustained()
{
    if (_sustainedName.empty())
        return;

    SDL_ClearAudioStream(_sustainedVoice); // drop queued data -> silence now
    _sustainedName.clear();
}

void AudioContext::playMusic(std::string_view filename, float volume)
{
    if (_musicName == filename)
        return; // already looping this track

    const Sound& sound = getSound(filename);
    SDL_ClearAudioStream(_musicVoice);
    SDL_SetAudioStreamFormat(_musicVoice, &sound.spec(), nullptr);
    SDL_SetAudioStreamGain(_musicVoice, volume);
    SDL_PutAudioStreamData(_musicVoice, sound.data(), static_cast<int>(sound.size()));
    _musicName = filename;
}

void AudioContext::stopMusic()
{
    if (_musicName.empty())
        return;

    SDL_ClearAudioStream(_musicVoice);
    _musicName.clear();
}

void AudioContext::updateMusic()
{
    if (_musicName.empty())
        return;

    // Loop: requeue a full copy whenever less than one copy remains buffered.
    const Sound& sound = getSound(_musicName);
    if (SDL_GetAudioStreamQueued(_musicVoice) < static_cast<int>(sound.size()))
        SDL_PutAudioStreamData(_musicVoice, sound.data(), static_cast<int>(sound.size()));
}

void AudioContext::startMicrowave(std::string_view filename, float volume)
{
    const Sound& sound = getSound(filename);
    SDL_ClearAudioStream(_microwaveVoice); // drop any previous cook's hum
    SDL_SetAudioStreamFormat(_microwaveVoice, &sound.spec(), nullptr);
    SDL_SetAudioStreamGain(_microwaveVoice, volume);
    SDL_PutAudioStreamData(_microwaveVoice, sound.data(), static_cast<int>(sound.size()));
}

void AudioContext::stopMicrowave()
{
    SDL_ClearAudioStream(_microwaveVoice); // cut the hum the instant cooking ends
}

void AudioContext::setVolume(float gain)
{
    SDL_SetAudioDeviceGain(_device, gain);
}
} // namespace cafe
