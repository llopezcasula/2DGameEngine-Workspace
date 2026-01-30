#pragma once

#include <string>
#include <memory>
#include <map>

// Forward declarations for miniaudio
struct ma_engine;
struct ma_sound;

class AudioManager
{
public:
    static void Init();
    static void Shutdown();
    static void Update();

    // Music control (looping background music)
    static void PlayMusic(const std::string& filepath, bool loop = true);
    static void StopMusic();
    static void PauseMusic();
    static void ResumeMusic();
    static bool IsMusicPlaying();

    // Sound effects (one-shot sounds)
    static void PlaySFX(const std::string& name, const std::string& filepath);
    static void PlaySFX(const std::string& name);  // Play pre-loaded sound
    
    // Pre-load sounds for better performance
    static void LoadSound(const std::string& name, const std::string& filepath);
    static void UnloadSound(const std::string& name);

    // Volume control (0.0 to 1.0)
    static void SetMasterVolume(float volume);
    static void SetMusicVolume(float volume);
    static void SetSFXVolume(float volume);
    
    static float GetMasterVolume() { return s_MasterVolume; }
    static float GetMusicVolume() { return s_MusicVolume; }
    static float GetSFXVolume() { return s_SFXVolume; }

    static void SetSoundLooping(const std::string& name, bool loop);
    static void StopSFX(const std::string& name);


    // Mute control
    static void SetMuted(bool muted);
    static bool IsMuted() { return s_Muted; }

private:
    struct AudioData;
    static std::unique_ptr<AudioData> s_Data;
    
    static float s_MasterVolume;
    static float s_MusicVolume;
    static float s_SFXVolume;
    static bool s_Muted;
};