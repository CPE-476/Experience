#ifndef SOUND_H
#define SOUND_H

#include <glad/glad.h>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "shader.h"
#include "camera.h"
using namespace std;
using namespace glm;

struct Sound {
    ma_sound sound;
    ma_sound_config soundConfig;
    ma_engine engine;
    ma_engine_config engineConfig;

    vec3 pos;
    float volume, rolloff, minDistance, maxDistance;
    bool isLooping, isDir, isMusic, hasPlayed;
    string path;

    Sound(string path, vec3 pos, float volume, float rolloff, float minDistance, 
            float maxDistance, bool isLooping, bool isMusic)
    {
        this->path = path;
        this->pos = pos;
        this->volume = volume;
        this->rolloff = rolloff;
        this->minDistance = minDistance;
        this->maxDistance = maxDistance;
        this->isLooping = isLooping;
        this->isDir = true;
        this->isMusic = isMusic;
        this->hasPlayed = false;
        setupSound();
    }

    Sound(string path, float volume, bool isLooping)
    {
        this->path = path;
        this->pos = vec3(0);
        this->volume = volume;
        this->rolloff = 1.0f;
        this->minDistance = 1.0f;
        this->isLooping = isLooping;
        this->isDir = false;
        this->isMusic = true;
        this->hasPlayed = false;
        setupSound();
    }

    void setupSound()
    {
        engineConfig = ma_engine_config_init();
        engineConfig.listenerCount = 1;
        if (ma_engine_init(&engineConfig, &engine) != MA_SUCCESS)
        {
            cout << "Failed to initialize audio engine.\n";
            return;
        }

        soundConfig = ma_sound_config_init();

        ma_engine_set_volume(&engine, volume);

        ma_sound_init_from_file(&engine, path.c_str(), MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &sound);

        if(!isDir)
            ma_sound_set_spatialization_enabled(&sound, false);

        if(!isMusic)
        {
            ma_engine_listener_set_position(&engine, 0, camera.Position.x, camera.Position.y, camera.Position.z);
            ma_engine_listener_set_direction(&engine, 0, camera.Front.x, camera.Front.y, camera.Front.z);
        }
        if(isMusic)
        {
            ma_engine_listener_set_direction(&engine, 0, pos.x, pos.y, pos.z);
        }
        
        ma_sound_set_position(&sound, pos.x, pos.y, pos.z);
        ma_sound_set_min_gain(&sound, 0.0f);
        ma_sound_set_rolloff(&sound, rolloff);
        ma_sound_set_min_distance(&sound, minDistance);
        ma_sound_set_min_distance(&sound, maxDistance);
        ma_sound_set_looping(&sound, isLooping);
    }

    void updateSound()
    {
        ma_engine_set_volume(&engine, volume);
        ma_sound_set_rolloff(&sound, rolloff);
        ma_sound_set_min_distance(&sound, minDistance);
        ma_sound_set_min_distance(&sound, maxDistance);
        startSound();

        ma_sound_set_position(&sound, pos.x, pos.y, pos.z);
        ma_engine_listener_set_position(&engine, 0, camera.Position.x, camera.Position.y, camera.Position.z);

        if(!isMusic)
        {
            ma_engine_listener_set_direction(&engine, 0, camera.Front.x, camera.Front.y, camera.Front.z);
            ma_engine_listener_set_world_up(&engine, 0, camera.WorldUp.x, camera.WorldUp.y, camera.WorldUp.z);
        }
    }

    void startSound()
    {
        if(!ma_sound_is_playing(&sound) && hasPlayed == false)
        {
            ma_sound_start(&sound);
        }

        /*
        if(!isLooping)
            hasPlayed = true;
        */
    }

    void stopSound()
    {
        ma_sound_stop(&sound);
    }

    void reset()
    {
        hasPlayed = false;
    }

    void setPitch(float pitch)
    {
        ma_sound_set_pitch(&sound, pitch);
    }
};

#endif
