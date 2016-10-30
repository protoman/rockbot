#ifndef SOUNDLIB_H
#define SOUNDLIB_H

#include <iostream>
using namespace std;

#include <SDL/SDL_mixer.h>
#include "defines.h"

/**
 * @brief
 *
 */
class soundLib
{
public:
/**
 * @brief
 *
 */
    soundLib();
    ~soundLib();
    void init_audio_system();
    void play_sfx(Uint8 sfx);
    void play_repeated_sfx(Uint8 sfx, Uint8 loops);
    void stop_repeated_sfx();
    bool is_playing_repeated_sfx() const;
    Uint8 get_repeated_sfx_n() const;
    void play_timed_sfx(Uint8 sfx, int time);
    void load_all_sfx();
    void load_music(string music_file);
    void load_boss_music(string music_file);
    void unload_music();
    void play_music();
    void play_boss_music();
    void load_stage_music (string filename);
    void restart_music();
    void stop_music() const;
    void close_audio();
    void sound_loop();
    void disable_sound() const;
    void enable_sound();

    void update_volumes();

    void play_sfx_from_file(std::string filename, int repeat_n);
    void play_shared_sfx(std::string filename);
    void play_sfx_from_chunk(Mix_Chunk* chunk, int repeat_n);
    Mix_Chunk *sfx_from_file(std::string filename);

private:


public:

private:
    Mix_Chunk *sfx_list[SFX_COUNT]; /**< TODO */
    Mix_Music *music; /**< TODO */
    Mix_Music *boss_music; /**< TODO */
    Sint8 _repeated_sfx_channel; /**< TODO */
    Uint8 _repeated_sfx; /**< TODO */
};

#endif // SOUNDLIB_H
