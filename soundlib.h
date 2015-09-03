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
    void play_sfx(int sfx);
    /**
     * @brief
     *
     * @param sfx
     * @param loops
     */
    void play_repeated_sfx(int sfx, int loops);
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
    int get_repeated_sfx_n() const;
    /**
     * @brief
     *
     * @param sfx
     * @param time
     */
    void play_timed_sfx(int sfx, int time);
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
    int _repeated_sfx_channel; /**< TODO */
    int _repeated_sfx; /**< TODO */
};

#endif // SOUNDLIB_H
