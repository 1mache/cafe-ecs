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

void AudioContext::play(std::string_view filename)
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
        SDL_PutAudioStreamData(voice, sound.data(), static_cast<int>(sound.size()));
        return;
    }
}

void AudioContext::startSustained(std::string_view filename, float startOffsetSeconds)
{
    if (_sustainedName == filename)
        return; // this sound already sustaining; don't restart it every frame

    const Sound& sound = getSound(filename);
    SDL_ClearAudioStream(_sustainedVoice); // drop whatever was sustaining before
    SDL_SetAudioStreamFormat(_sustainedVoice, &sound.spec(), nullptr);

    // Skip startOffsetSeconds into the file. Offset is frame-aligned by construction
    // (whole frames * frame size), so playback stays sample-correct.
    Uint32 offset = static_cast<Uint32>(startOffsetSeconds * static_cast<float>(sound.spec().freq))
                  * SDL_AUDIO_FRAMESIZE(sound.spec());
    if (offset > sound.size()) offset = sound.size();

    SDL_PutAudioStreamData(_sustainedVoice, sound.data() + offset,
                           static_cast<int>(sound.size() - offset));
    _sustainedName = filename;
}

void AudioContext::stopSustained()
{
    if (_sustainedName.empty())
        return;

    SDL_ClearAudioStream(_sustainedVoice); // drop queued data -> silence now
    _sustainedName.clear();
}

void AudioContext::setVolume(float gain)
{
    SDL_SetAudioDeviceGain(_device, gain);
}
} // namespace cafe
