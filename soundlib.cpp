#include <cstdlib>
#include "soundlib.h"

#ifdef ANDROID
#include <android/log.h>
#endif


extern string FILEPATH;
extern string GAMEPATH;

#include "file/format.h"
#include "aux_tools/stringutils.h"

soundLib::soundLib() : _repeated_sfx_channel(-1), _repeated_sfx(-1)
{
	music = NULL;
	boss_music = NULL;
    is_playing_boss_music = false;

    //game_config.volume_music = 10;
    //game_config.volume_sfx = 100;
}

soundLib::~soundLib()
{
	close_audio();
}

void soundLib::init_audio_system()
{
    int bitrate = MIX_DEFAULT_FREQUENCY;
    int channels = MIX_DEFAULT_CHANNELS;
#if defined(ANDROID) || defined(POCKETGO)
    //bitrate = 11000;
    //channels = 1;
#endif
    if (Mix_OpenAudio(bitrate, MIX_DEFAULT_FORMAT, channels, 1024) < 0) {
        std::cout << "Couldn't open audio. Error: " << SDL_GetError() << std::endl;
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### SOUNDLIB[Couldn't open audio.] ###");
#endif
    }
    Mix_Volume(-1, SharedData::get_instance()->game_config.volume_sfx);
    Mix_VolumeMusic(SharedData::get_instance()->game_config.volume_music);
	load_all_sfx();
}


void soundLib::play_sfx(Uint8 sfx) {
    if (SharedData::get_instance()->game_config.sound_enabled == false) {
        return;
	}

	if (sfx_list[sfx] != NULL) {
        Mix_PlayChannel(-1, sfx_list[sfx], 0);
	}
}

void soundLib::play_repeated_sfx(Uint8 sfx, Uint8 loops) {
    if (SharedData::get_instance()->game_config.sound_enabled == false) {
        return;
	}

    if (sfx_list[sfx] != NULL) {
		if (is_playing_repeated_sfx()) {
			stop_repeated_sfx();
		}
		_repeated_sfx = sfx;
		_repeated_sfx_channel = Mix_PlayChannel(-1, sfx_list[sfx], loops);
    } else {
        cout << "Error: soundLib::play_sfx - null sfx\n";
	}
}

void soundLib::stop_repeated_sfx()
{
    if (_repeated_sfx_channel == -1) {
        return;
    }
	Mix_HaltChannel(_repeated_sfx_channel);
	_repeated_sfx = -1;
	_repeated_sfx_channel = -1;
}

bool soundLib::is_playing_repeated_sfx() const
{
	if (_repeated_sfx_channel == -1) {
		return false;
	}
	return true;
}

Uint8 soundLib::get_repeated_sfx_n() const
{
	return _repeated_sfx;
}


void soundLib::play_timed_sfx(Uint8 sfx, int time) {
    if (SharedData::get_instance()->game_config.sound_enabled == false) {
        return;
	}

	if (sfx_list[sfx] != NULL) {
		Mix_PlayChannelTimed(-1, sfx_list[sfx], -1 , time);
	}
}

// must follow the oders defined in enum SFX_LIST (types.h)
void soundLib::load_all_sfx() {
	int i = 0;
	string filename;

    filename = FILEPATH + "sfx/npc_hit.wav";
	sfx_list[i] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/npc_killed.wav";
	sfx_list[i] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/player_hit.wav";
	sfx_list[i] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/player_shot.wav";
	sfx_list[i] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/player_jump.wav";
	sfx_list[i] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/cursor.wav";
	sfx_list[i] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/stage_selected.wav";
	sfx_list[i] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/got_energy_pill.wav";
	sfx_list[i] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/got_item.wav";
	sfx_list[i] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/shot_reflected.wav";
	sfx_list[i] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/door_open.wav";
	sfx_list[i] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/got_weapon.wav";
	sfx_list[i] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/teleport.wav";
	sfx_list[i] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/implosion.wav";
	sfx_list[i] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/player_death.wav";
	sfx_list[i] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/destrin_ship.wav";
	sfx_list[i] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/charged_shot.wav";
	sfx_list[SFX_PLAYER_CHARGED_SHOT] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/charging1.wav";
	sfx_list[SFX_CHARGING1] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/charging2.wav";
	sfx_list[SFX_CHARGING2] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/big_explosion.wav";
	sfx_list[SFX_BIG_EXPLOSION] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/water_enter.wav";
	sfx_list[SFX_WATER_ENTER] = Mix_LoadWAV(filename.c_str());
	i++;

    filename = FILEPATH + "sfx/water_leave.wav";
	sfx_list[SFX_WATER_LEAVE] = Mix_LoadWAV(filename.c_str());
	i++;


    filename = FILEPATH + "sfx/disappearning_block.wav";
    sfx_list[SFX_DISAPPEARING_BLOCK] = Mix_LoadWAV(filename.c_str());
    i++;

    filename = FILEPATH + "sfx/shoryuken_girl.wav";
    sfx_list[SFX_SHORYUKEN_GIRL] = Mix_LoadWAV(filename.c_str());
    i++;

    filename = FILEPATH + "sfx/beam.wav";
    sfx_list[SFX_BEAM] = Mix_LoadWAV(filename.c_str());
    i++;


    filename = FILEPATH + "sfx/recharge.wav";
    sfx_list[SFX_GOT_ENERGY_BIG] = Mix_LoadWAV(filename.c_str());
    i++;
}



void soundLib::load_music(std::string music_file) {
	string filename;

    unload_music();
    filename = FILEPATH + "music/" + music_file;
    filename = get_filename_for_music(filename);
	music = Mix_LoadMUS(filename.c_str());
	if (!music) {
        std::cout << "Error in soundLib::load_music::Mix_LoadMUS('" << filename << "': '" << Mix_GetError() << "'\n";
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### SOUNDLIB::load_music - not found[%s] ###", music_file.c_str());
#endif
    }
}

void soundLib::load_shared_music(string music_file)
{
    string filename;

    unload_music();
    filename = GAMEPATH + "/shared/music/" + music_file;
    filename = get_filename_for_music(filename);
    music = Mix_LoadMUS(filename.c_str());
    if (!music) {
        std::cout << "Error in soundLib::load_music::Mix_LoadMUS('" << filename << "': '" << Mix_GetError() << "'\n";
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### SOUNDLIB::load_music - not found[%s] ###", music_file.c_str());
#endif
    }
}

void soundLib::load_boss_music(string music_file) {
	string filename;

	if (boss_music != NULL) {
		Mix_HaltMusic();
		Mix_FreeMusic(boss_music);
        boss_music = NULL;
	}
    filename = FILEPATH + "music/" + music_file;
    filename = get_filename_for_music(filename);
	boss_music = Mix_LoadMUS(filename.c_str());
	if (!boss_music) {
        std::cout << "Error in soundLib::load_boss_music::Mix_LoadMUS('" << filename << "': '" << Mix_GetError() << "'\n";
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### SOUNDLIB::load_boss_music - not found[%s] ###", music_file.c_str());
#endif
    }
}

void soundLib::unload_music()
{
	if (music != NULL) {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
		music = NULL;
	}
    is_playing_boss_music = false;
}



void soundLib::play_music() {

    if (SharedData::get_instance()->game_config.sound_enabled == false) {
        return;
	}
    int res = -1;
	// toca a música
	if (music) {
        res = Mix_PlayMusic(music, -1);
        if (res == -1) {
            std::cout << "ERROR: Mix_PlayMusic Error: " << Mix_GetError() << std::endl;
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### Mix_PlayMusic Error[%s] ###", Mix_GetError());
#endif
		}
        Mix_VolumeMusic(SharedData::get_instance()->game_config.volume_music);
	} else {
        std::cout << "ERROR: play_music, music is null" << std::endl;
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### SOUNDLIB::play_music - music is NULL ###");
#endif
    }
}

void soundLib::play_music_once()
{
    if (SharedData::get_instance()->game_config.sound_enabled == false) {
        return;
    }
    int res = -1;
    if (music) {
        res = Mix_PlayMusic(music, 1);
        Mix_VolumeMusic(SharedData::get_instance()->game_config.volume_music);
    } else {
        std::cout << ">> soundLib::play_music_once: music is null" << std::endl;
    }
}

void soundLib::play_boss_music() {
    is_playing_boss_music = true;
    if (SharedData::get_instance()->game_config.sound_enabled == false) {
        return;
	}
	// toca a música
	if (boss_music) {
		if (Mix_PlayMusic(boss_music, -1) == -1) {
            std::cout << "<<<<<<<<<<<<< Mix_PlayMusic, Error: " << Mix_GetError() << std::endl;
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### SOUNDLIB::play_boss_music Error[%s] ###", Mix_GetError());
#endif
		}
        Mix_VolumeMusic(SharedData::get_instance()->game_config.volume_music);
	} else {
		printf(">> play_boss_music ERROR: boss_music is null\n");
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### SOUNDLIB::play_boss_music - music is NULL ###");
#endif
    }
}

void soundLib::load_stage_music(std::string filename) {
    is_playing_boss_music = false;
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### SOUNDLIB::load_stage_music[%s] ###", filename.c_str());
#endif
    Mix_HaltMusic();
    if (filename.length() > 0) {
        load_music(filename);
	} else {
        std::cout << "soundLib::load_stage_music - music filename undefined." << std::endl;
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### SOUNDLIB::load_stage_music - music is NULL ###");
#endif
    }
}

void soundLib::restart_music()
{
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### SOUNDLIB::restart_music");
#endif
    Mix_HaltMusic();
    play_music();
}


void soundLib::stop_music() const {
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### SOUNDLIB::stop_music");
#endif
    Mix_HaltMusic();
}

void soundLib::close_audio() {
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### SOUNDLIB::close_audio");
#endif
    Mix_FreeMusic(music);
	Mix_CloseAudio();
}

void sound_loop() {}

void soundLib::disable_sound() const
{
    SharedData::get_instance()->game_config.sound_enabled = false;
	stop_music();
}

void soundLib::enable_sound()
{
    SharedData::get_instance()->game_config.sound_enabled = true;
    play_music();
}

string soundLib::get_filename_for_music(string filename)
{
    if (SharedData::get_instance()->game_config.old_old_style_music == 1) {
        std::size_t found = filename.find_last_of("/");
        if (found != std::string::npos) {
            std::string old_filename = filename.substr(0,found) + "/old/" + filename.substr(found+1);
            if (fio.file_exists(old_filename)) {
                filename = old_filename;
            }
        }
    } else {
#if defined(ANDROID)
        std::size_t found = filename.find_last_of("/");
        if (found != std::string::npos) {
            std::string mp3_filename = filename.substr(0,found) + "/mp3/" + filename.substr(found+1) + ".mp3";
            if (fio.file_exists(mp3_filename)) {
                filename = mp3_filename;
            }
        }
#endif
    }
    filename = StringUtils::clean_filename(filename);
    return filename;
}

void soundLib::update_volumes()
{
    Mix_VolumeMusic(SharedData::get_instance()->game_config.volume_music);
    Mix_Volume(-1, SharedData::get_instance()->game_config.volume_sfx);
}

void soundLib::play_sfx_from_file(string filename, int repeat_n)
{
    if (SharedData::get_instance()->game_config.sound_enabled == false) {
        return;
    }
    filename = FILEPATH + "/sfx/" + filename;
    filename = StringUtils::clean_filename(filename);
    Mix_Chunk *sfx = Mix_LoadWAV(filename.c_str());
    if (!sfx) {
        return;
    }
    Mix_PlayChannel(-1, sfx, repeat_n-1);
}

void soundLib::play_shared_sfx(string filename)
{
    if (SharedData::get_instance()->game_config.sound_enabled == false) {
        return;
    }
    filename = GAMEPATH + "/shared/sfx/" + filename;
    filename = StringUtils::clean_filename(filename);
    Mix_Chunk *sfx = Mix_LoadWAV(filename.c_str());

    if (!sfx) {
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### SOUNDLIB::play_shared_sfx - error loading [%s] ###", filename.c_str());
#endif
        return;
    }

    Mix_PlayChannel(-1, sfx, 0);
}

void soundLib::play_sfx_from_chunk(Mix_Chunk *chunk, int repeat_n)
{
    if (SharedData::get_instance()->game_config.sound_enabled == false) {
        return;
    }
    if (!chunk) {
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### SOUNDLIB::play_sfx_from_chunk - invalid chunk ###");
#endif
        return;
    }
    Mix_PlayChannel(-1, chunk, repeat_n-1);
}

Mix_Chunk* soundLib::sfx_from_file(string filename)
{
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "### SOUNDLIB::sfx_from_file[%s] ###", filename.c_str());
#endif
    filename = FILEPATH + "/sfx/" + filename;
    filename = StringUtils::clean_filename(filename);
    Mix_Chunk *sfx = Mix_LoadWAV(filename.c_str());
    return sfx;
}

bool soundLib::get_is_playing_boss_music()
{
    return is_playing_boss_music;
}

