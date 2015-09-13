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
    /**
     * @brief
     *
     */
    ~soundLib();
    /**
     * @brief
     *
     */
    void init_audio_system();
    /**
     * @brief
     *
     * @param sfx
     */
    void play_sfx(Uint8 sfx);
    /**
     * @brief
     *
     * @param sfx
     * @param loops
     */
    void play_repeated_sfx(Uint8 sfx, Uint8 loops);
    /**
     * @brief
     *
     */
    void stop_repeated_sfx();
    /**
     * @brief
     *
     * @return bool
     */
    bool is_playing_repeated_sfx() const;
    /**
     * @brief
     *
     * @return int
     */
    Uint8 get_repeated_sfx_n() const;
    /**
     * @brief
     *
     * @param sfx
     * @param time
     */
    void play_timed_sfx(Uint8 sfx, int time);
    /**
     * @brief
     *
     */
    void load_all_sfx();

    /**
     * @brief
     *
     * @param music_file
     */
    void load_music(string music_file);
    /**
     * @brief
     *
     * @param music_file
     */
    void load_boss_music(string music_file);
    /**
     * @brief
     *
     */
    void unload_music();
    /**
     * @brief
     *
     */
    void play_music();
    /**
     * @brief
     *
     */
    void play_boss_music();
    /**
     * @brief
     *
     * @param filename
     */
    void load_stage_music (string filename);
    /**
     * @brief
     *
     */
    void restart_music();
    /**
     * @brief
     *
     */
    void stop_music() const;
    /**
     * @brief
     *
     */
    void close_audio();
    /**
     * @brief
     *
     */
    void sound_loop();

    /**
     * @brief
     *
     */
    void disable_sound() const;
    /**
     * @brief
     *
     */
    void enable_sound();

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
