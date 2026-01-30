#include "Audio/AudioManager.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

struct LoadedSound
{
    ma_sound sound;
    std::string filepath;
    bool isLoaded;
};

struct AudioManager::AudioData
{
    ma_engine engine;
    ma_sound* currentMusic = nullptr;
    std::map<std::string, LoadedSound> loadedSounds;
    bool initialized = false;
};

std::unique_ptr<AudioManager::AudioData> AudioManager::s_Data = nullptr;
float AudioManager::s_MasterVolume = 1.0f;
float AudioManager::s_MusicVolume = 0.5f;
float AudioManager::s_SFXVolume = 0.7f;
bool AudioManager::s_Muted = false;

void AudioManager::Init()
{
    if (s_Data && s_Data->initialized)
        return; // already initialized

    s_Data = std::make_unique<AudioData>();

    ma_result result = ma_engine_init(NULL, &s_Data->engine);
    if (result != MA_SUCCESS)
    {
        std::cerr << "Failed to initialize audio engine\n";
        s_Data.reset();
        return;
    }

    s_Data->initialized = true;
    ma_engine_set_volume(&s_Data->engine, s_MasterVolume);

    std::cout << "Audio system initialized successfully\n";
    std::cout << "Supported formats: WAV, MP3, FLAC, OGG\n";
}

void AudioManager::Shutdown()
{
    if (!s_Data || !s_Data->initialized) return;

    // Stop and cleanup current music
    if (s_Data->currentMusic)
    {
        ma_sound_uninit(s_Data->currentMusic);
        delete s_Data->currentMusic;
        s_Data->currentMusic = nullptr;
    }

    // Cleanup all loaded sounds
    for (auto& pair : s_Data->loadedSounds)
    {
        if (pair.second.isLoaded)
        {
            ma_sound_uninit(&pair.second.sound);
        }
    }
    s_Data->loadedSounds.clear();

    // Uninitialize engine
    ma_engine_uninit(&s_Data->engine);
    
    s_Data.reset();
    std::cout << "Audio system shut down\n";
}

void AudioManager::Update()
{
    // miniaudio handles updates internally, but we can add custom logic here if needed
}

void AudioManager::PlayMusic(const std::string& filepath, bool loop)
{
    if (!s_Data || !s_Data->initialized)
    {
        std::cerr << "Audio system not initialized\n";
        return;
    }

    // Stop current music if playing
    StopMusic();

    // Create new music sound
    s_Data->currentMusic = new ma_sound();
    
    ma_result result = ma_sound_init_from_file(
        &s_Data->engine,
        filepath.c_str(),
        MA_SOUND_FLAG_STREAM,  // Stream for music (doesn't load entire file)
        NULL,
        NULL,
        s_Data->currentMusic
    );

    if (result != MA_SUCCESS)
    {
        std::cerr << "Failed to load music: " << filepath << "\n";
        delete s_Data->currentMusic;
        s_Data->currentMusic = nullptr;
        return;
    }

    // Set looping
    ma_sound_set_looping(s_Data->currentMusic, loop ? MA_TRUE : MA_FALSE);
    
    // Set volume
    ma_sound_set_volume(s_Data->currentMusic, s_MusicVolume * s_MasterVolume);

    // Start playing
    ma_sound_start(s_Data->currentMusic);

    std::cout << "Playing music: " << filepath << (loop ? " (looping)" : "") << "\n";
}

void AudioManager::StopMusic()
{
    if (!s_Data || !s_Data->currentMusic) return;

    ma_sound_stop(s_Data->currentMusic);
    ma_sound_uninit(s_Data->currentMusic);
    delete s_Data->currentMusic;
    s_Data->currentMusic = nullptr;
}

void AudioManager::PauseMusic()
{
    if (!s_Data || !s_Data->currentMusic) return;
    ma_sound_stop(s_Data->currentMusic);
}

void AudioManager::ResumeMusic()
{
    if (!s_Data || !s_Data->currentMusic) return;
    ma_sound_start(s_Data->currentMusic);
}

bool AudioManager::IsMusicPlaying()
{
    if (!s_Data || !s_Data->currentMusic) return false;
    return ma_sound_is_playing(s_Data->currentMusic);
}

void AudioManager::LoadSound(const std::string& name, const std::string& filepath)
{
    if (!s_Data || !s_Data->initialized)
    {
        std::cerr << "Audio system not initialized\n";
        return;
    }

    if (s_Data->loadedSounds.find(name) != s_Data->loadedSounds.end())
    {
        std::cerr << "Sound '" << name << "' already loaded\n";
        return;
    }


    // Create the map entry first (no copying of ma_sound)
    auto [it, inserted] = s_Data->loadedSounds.emplace(name, LoadedSound{});
    LoadedSound& ls = it->second;

    ls.filepath = filepath;
    ls.isLoaded = false;

    ma_result result = ma_sound_init_from_file(
        &s_Data->engine,
        filepath.c_str(),
        0,
        nullptr,
        nullptr,
        &ls.sound
    );

    if (result != MA_SUCCESS)
    {
        std::cerr << "Failed to load sound: " << filepath << "\n";
        s_Data->loadedSounds.erase(it);
        return;
    }

    ls.isLoaded = true;
    ma_sound_set_volume(&ls.sound, (s_Muted ? 0.0f : (s_SFXVolume * s_MasterVolume)));

    std::cout << "Loaded sound: " << name << " from " << filepath << "\n";
}

void AudioManager::UnloadSound(const std::string& name)
{
    if (!s_Data) return;

    auto it = s_Data->loadedSounds.find(name);
    if (it != s_Data->loadedSounds.end())
    {
        if (it->second.isLoaded)
        {
            ma_sound_uninit(&it->second.sound);
        }
        s_Data->loadedSounds.erase(it);
        std::cout << "Unloaded sound: " << name << "\n";
    }
}

void AudioManager::PlaySFX(const std::string& name, const std::string& filepath)
{
    // Load and play in one call
    LoadSound(name, filepath);
    PlaySFX(name);
}

void AudioManager::PlaySFX(const std::string& name)
{
    if (!s_Data || !s_Data->initialized) return;

    auto it = s_Data->loadedSounds.find(name);
    if (it == s_Data->loadedSounds.end())
    {
        std::cerr << "Sound '" << name << "' not loaded\n";
        return;
    }

    if (!it->second.isLoaded) return;

    // Seek to start and play
    ma_sound_seek_to_pcm_frame(&it->second.sound, 0);
    ma_sound_start(&it->second.sound);
}

void AudioManager::SetMasterVolume(float volume)
{
    s_MasterVolume = glm::clamp(volume, 0.0f, 1.0f);
    
    if (s_Data && s_Data->initialized)
    {
        float effectiveVolume = s_Muted ? 0.0f : s_MasterVolume;
        ma_engine_set_volume(&s_Data->engine, effectiveVolume);
    }
}

void AudioManager::SetMusicVolume(float volume)
{
    s_MusicVolume = glm::clamp(volume, 0.0f, 1.0f);
    
    if (s_Data && s_Data->currentMusic)
    {
        float effectiveVolume = s_Muted ? 0.0f : (s_MusicVolume * s_MasterVolume);
        ma_sound_set_volume(s_Data->currentMusic, effectiveVolume);
    }
}

void AudioManager::SetSFXVolume(float volume)
{
    s_SFXVolume = glm::clamp(volume, 0.0f, 1.0f);
    
    if (s_Data)
    {
        float effectiveVolume = s_Muted ? 0.0f : (s_SFXVolume * s_MasterVolume);
        
        // Update all loaded sounds
        for (auto& pair : s_Data->loadedSounds)
        {
            if (pair.second.isLoaded)
            {
                ma_sound_set_volume(&pair.second.sound, effectiveVolume);
            }
        }
    }
}

void AudioManager::SetSoundLooping(const std::string& name, bool loop)
{
    if (!s_Data || !s_Data->initialized) return;

    auto it = s_Data->loadedSounds.find(name);
    if (it == s_Data->loadedSounds.end() || !it->second.isLoaded) return;

    ma_sound_set_looping(&it->second.sound, loop ? MA_TRUE : MA_FALSE);
}

void AudioManager::StopSFX(const std::string& name)
{
    if (!s_Data || !s_Data->initialized) return;

    auto it = s_Data->loadedSounds.find(name);
    if (it == s_Data->loadedSounds.end() || !it->second.isLoaded) return;

    ma_sound_stop(&it->second.sound);
}


void AudioManager::SetMuted(bool muted)
{
    s_Muted = muted;
    
    // Refresh all volumes
    SetMasterVolume(s_MasterVolume);
    SetMusicVolume(s_MusicVolume);
    SetSFXVolume(s_SFXVolume);
}