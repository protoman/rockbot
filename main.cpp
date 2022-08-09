#ifdef WII
#include <fat.h>
#endif

#ifdef PSP
#include <pspkernel.h>
#include <psppower.h>
#endif

#ifdef ANDROID
#include <jni.h>
#include <android/log.h>
#include <SDL_screenkeyboard.h>
#include "ports/android/android_game_services.h"
#elif OSX
#include <mach-o/dyld.h>
#elif DREAMCAST
#include <kos.h>
#endif

#include <iostream>
#include <SDL/SDL.h>				//Include da SDL
#include <SDL/SDL_image.h>		//Include da biblioteca SDL_Image
#include <SDL/SDL_mixer.h>	// Include da biblioteca SDL_Mixer
#include <SDL/SDL_ttf.h>		// Include da biblioteca SDL_ttf
#include <SDL/SDL_endian.h>
#include <SDL/SDL_mixer.h>

#include "strings_map.h"

// ************** CODE AND DEBUG flags ************** //
//#define PS2LOADFROMFIXEDPOINT 1
//#define DISABLESOUND 1
#define PS2LINK 1

// GLOBAL/EXTERN VARIABLES
std::string GAMEPATH; // path without DATA/GAMES
std::string FILEPATH; // path including game-data dir
std::string SAVEPATH;
std::string GAMENAME; // the gamename, part of path
SDL_Event event;
bool run_game = true;

#ifdef ANDROID
jobject activity_ref;
#endif

#include "defines.h"
#include "aux_tools/stringutils.h"

#include "graphicslib.h"
#include "graphic/draw.h"
#include "inputlib.h"
#include "timerlib.h"
#include "soundlib.h"
#include "game.h"
#include "stage_select.h"

#define MAXPATHLEN 1024

#if defined(LINUX) || defined(OSX) || defined(RASPBERRY)
    #include <errno.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <sys/param.h>
#elif defined(PLAYSTATION2) || defined(DINGUX) || defined(PSP) || defined(ANDROID) || defined(OPEN_PANDORA) || defined(WII) || defined(POCKETGO)
    #include <unistd.h>
#elif defined(WIN32)
    #include <direct.h>
    #undef main // to build on win32
#endif

#if defined(WIN32)
    std::string EXEC_NAME = std::string("rockbot.exe");
#elif defined(ANDROID)
    std::string EXEC_NAME("");
#elif defined(PLAYSTATION2)
    std::string EXEC_NAME("rockbot.elf");
#elif defined(PSP)
    std::string EXEC_NAME("EBOOT.PBP");
#elif defined(WII)
    std::string EXEC_NAME("boot.dol");
#elif defined(DINGUX)
    std::string EXEC_NAME("rockbot.dge");
#elif defined(POCKETGO)
    std::string EXEC_NAME("rockbot_pocketgo");
#else
    std::string EXEC_NAME("rockbot");
#endif

// EXTERNAL GLOBALS
timerLib timer;
inputLib input;
soundLib soundManager;
graphicsLib graphLib;
draw draw_lib;
game gameControl;
CURRENT_FILE_FORMAT::st_save game_save;
bool GAME_FLAGS[FLAG_COUNT];
#ifdef ANDROID
android_game_services game_services;
#endif


int default_keys_codes[BTN_COUNT]; // number indicator for the keyboard-keys
int default_button_codes[BTN_COUNT]; // number indicator for the keyboard-keys

bool leave_game = false;

#include "file/file_io.h"
CURRENT_FILE_FORMAT::file_io fio;
CURRENT_FILE_FORMAT::file_game game_data;
CURRENT_FILE_FORMAT::file_stage stage_data;

#include "defines.h"


FREEZE_EFFECT_TYPES freeze_weapon_effect = FREEZE_EFFECT_NONE;
int freeze_weapon_id = -1;

struct CURRENT_FILE_FORMAT::st_checkpoint checkpoint;

int current_stage = -1;

#ifdef PLAYSTATION2
    #include "ports/ps2/modules.h"


void PS2_copy_to_memorycard(std::string file_in, std::string file_out)
{
    FILE *fp_read = fopen(file_in.c_str(), "rb");
    if (!fp_read) {
        std::cout << "ERROR: Could not open read file '" << file_in << "'" << std::endl;
        graphLib.show_debug_msg("ERROR #1");
        graphLib.show_debug_msg(file_in);
        return;
    } else {
        std::cout << "WARNING: Opened file '" << file_in << "'" << std::endl;
    }
    file_out = SAVEPATH + "/" + file_out;
    FILE *fp_write = fopen(file_out.c_str(), "wb");
    if (!fp_write) {
        std::cout << "ERROR: Could not open write file '" << file_out << "'" << std::endl;
        graphLib.show_debug_msg("ERROR #2");
        graphLib.show_debug_msg(file_out);
        return;
    } else {
        std::cout << "WARNING: Opened file '" << file_out << "'" << std::endl;
    }

    int a;
    while(1)
    {
        a  =  fgetc(fp_read);
        if (!feof(fp_read)) {
            fputc(a, fp_write);
        } else {
            break;
        }
    }

    fclose(fp_read);
    fclose(fp_write);

}

void PS2_create_save_icons()
{
    // copy save icon files, if they do not exist
    std::string filename = SAVEPATH + "/icon.sys";
    FILE *fp = fopen(filename.c_str(), "rb");
    if (fp == NULL) {
        std::string filename = GAMEPATH + "/shared/images/icon.sys";
        PS2_copy_to_memorycard(filename, "icon.sys");
        filename = GAMEPATH + "/shared/images/rockbot_ps2_icon.icn";
        PS2_copy_to_memorycard(filename, "rockbot_ps2_icon.icn");
    } else {
        fclose(fp);
    }
}



#endif


#ifdef PSP
PSP_MODULE_INFO("Rockbot", PSP_MODULE_USER, 1, 0);
PSP_HEAP_SIZE_KB(-1024);
//PSP_HEAP_SIZE_MAX();


/* Exit callback */
int exit_callback(int arg1, int arg2, void *common) {
    sceKernelExitGame();
    return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp) {
    int cbid;
    cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void) {
    int thid = 0;
    thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if(thid >= 0) {
        sceKernelStartThread(thid, 0, 0);
    }
    return thid;
}


// LINKER PATCH
/*
extern "C" {
void *__dso_handle = NULL;
}
*/

// ram counter object
#endif


void get_filepath()
{
#ifdef WIN32
    char* buffer;
    if( (buffer = _getcwd( NULL, 0 )) != NULL ) {
        FILEPATH = std::string(buffer);
        FILEPATH += "/";
    }
    delete[] buffer;
#elif defined OSX
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        FILEPATH = std::string(path);
        FILEPATH = FILEPATH.substr(0, FILEPATH.length()-7);
    }
#else
    char *buffer = new char[MAXPATHLEN];
    char *res = getcwd(buffer, MAXPATHLEN);

#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "MAIN.DEBUG.res[%s]",res);
#endif

    UNUSED(res);
    if(buffer != NULL){
        FILEPATH = std::string(buffer);
    }
    FILEPATH += "/";
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "MAIN.DEBUG.FILEPATH[%s]",FILEPATH.c_str());
#endif

    delete[] buffer;
#endif

#if defined(PLAYSTATION2) && defined(PS2LOADFROMFIXEDPOINT) // DEBUG
    FILEPATH = "mass:/PS2/Rockbot/";
#elif DREAMCAST
    FILEPATH = "/cd/";
#endif

    GAMEPATH = FILEPATH;
}


void detect_language() {
    if (SharedData::get_instance()->game_config.selected_language == LANGUAGE_AUTODETECT) {
        // try to get language from the env, if set
        if (const char* env_lang = std::getenv("LANGUAGE")) {
            std::string lang_str(env_lang);
            std::string language = "en";
            if (std::string::npos != lang_str.find(":")) {
                std::vector<std::string> lang_list = StringUtils::split(lang_str, ":");
                if (lang_list.size() > 0) {
                    language = lang_list.at(0);
                }
            } else {
                language = lang_str;
            }
            if (language == "pt" || language.find("pt_") != std::string::npos || language.find("pt-") != std::string::npos) {
                SharedData::get_instance()->current_language = LANGUAGE_PORTUGUESE;
            } else if (language == "es" || language.find("es_") != std::string::npos || language.find("es-") != std::string::npos) {
                SharedData::get_instance()->current_language = LANGUAGE_SPANISH;
            } else { // default fallback
                SharedData::get_instance()->current_language = LANGUAGE_ENGLISH;
            }
        } else { // default fallback
            SharedData::get_instance()->current_language = LANGUAGE_ENGLISH;
        }
    } else {
        SharedData::get_instance()->current_language = SharedData::get_instance()->game_config.selected_language;
    }
}

int main(int argc, char *argv[])
{

#ifdef PSP
    SetupCallbacks();
    scePowerSetClockFrequency(333, 333, 166);
#endif
    for (int i=0; i<FLAG_COUNT; i++) {
		GAME_FLAGS[i] = false;
	}
	UNUSED(argc);

    string argvString = "";
    argvString = string(argv[0]);


    fflush(stdout);

    get_filepath();
    // fallback in case getcwd returns null
    if (FILEPATH.size() == 0) {
        FILEPATH = argvString.substr(0, argvString.size()-EXEC_NAME.size());
    }

#ifdef PLAYSTATION2
    PS2_init();

    // --- DEBUG --- //
    //FILEPATH = "cdfs:/";
    // --- DEBUG --- //

    //PS2_load_xio();

    if (FILEPATH.find("mass:") != std::string::npos) {
        PS2_load_USB();
    }

    if (FILEPATH.find("cdfs") != std::string::npos || FILEPATH.find("cdrom") != std::string::npos) {
        FILEPATH = "cdfs:";
        PS2_load_CDROM();
    }
#endif


#ifdef PC
	// check command-line paramethers
	if (argc > 1) {
		for (int i=1; i<argc; i++) {
			std::string temp_argv(argv[i]);
			if (temp_argv == "--fullscreen") {
                // TODO
			} else if (temp_argv == "--quickload") {
				GAME_FLAGS[FLAG_QUICKLOAD] = true;
            } else if (temp_argv == "--invencible") { // player is not hit by enemies
				GAME_FLAGS[FLAG_INVENCIBLE] = true;
			} else if (temp_argv == "--allweapons") { // player have all weapons available even if
				GAME_FLAGS[FLAG_ALLWEAPONS] = true;
			} else if (temp_argv == "--infinitejump") { // player can jump again and again
				GAME_FLAGS[FLAG_INFINITE_JUMP] = true;
            } else if (temp_argv == "--player1") {
                GAME_FLAGS[FLAG_PLAYER1] = true;
            } else if (temp_argv == "--player2") {
                GAME_FLAGS[FLAG_PLAYER2] = true;
            } else if (temp_argv == "--player3") {
                GAME_FLAGS[FLAG_PLAYER3] = true;
            } else if (temp_argv == "--player4") {
                GAME_FLAGS[FLAG_PLAYER4] = true;
            } else if (temp_argv == "--gamename" && argc >= i+1) {
                std::string game_name(argv[i+1]);
                gameControl.set_selected_game(game_name);
            } else if (temp_argv == "--stage" && argc >= i+1) {
                std::string stage_n_str(argv[i+1]);
                sscanf(stage_n_str.c_str(), "%d", &current_stage);
            }
		}
	}
#endif
    fflush(stdout);

    /// DEBUG ///
    //GAME_FLAGS[FLAG_QUICKLOAD] = true;
    //GAME_FLAGS[FLAG_INVENCIBLE] = true;

    // PS2 version have to load config AFTER SDL_Init due to SDK issues
    #if defined(LINUX) || defined(OSX) || defined(RASPBERRY)
        SAVEPATH = std::string(getenv("HOME")) + "/.rockdroid/";
        mkdir(SAVEPATH.c_str(), 0777);
    #elif WIN32
        SAVEPATH =  std::string(getenv("APPDATA")) + "/rockdroid";
        _mkdir(SAVEPATH.c_str());
    #else
        SAVEPATH = GAMEPATH;
    #endif

    fflush(stdout);


#ifndef PLAYSTATION2
    fio.load_config(SharedData::get_instance()->game_config);
#endif

    detect_language();


    // INIT GRAPHICS
    if (graphLib.initGraphics() != true) {
        fflush(stdout);
        return -1;
    }

    fflush(stdout);

#ifdef ANDROID
    game_services.init_android_button_size();
    game_services.set_android_default_buttons_size(SharedData::get_instance()->game_config.android_touch_controls_size, SharedData::get_instance()->game_config.android_button_spacing);
    game_services.set_touch_controls_visible(!SharedData::get_instance()->game_config.android_touch_controls_hide);
#endif


    //GAMENAME = std::string("RockBot");
    gameControl.select_game_screen();
    GAMENAME = gameControl.get_selected_game();

    fflush(stdout);


    // DEBUG PS2 //
    //GAMENAME = std::string("RockBot");

    if (GAMENAME == "") {
        graphLib.draw_text(20, 20, strings_map::get_instance()->get_ingame_string(strings_ingame_engineerror) + std::string(":"));
        graphLib.draw_text(20, 32, strings_map::get_instance()->get_ingame_string(strings_ingame_nogames));

        std::string filename = GAMEPATH + "/games/";
        filename = StringUtils::clean_filename(filename);

        graphLib.draw_text(20, 44, filename);

        input.wait_keypress();
        SDL_Quit();
        return 0;
    }
    FILEPATH += std::string("/games/") + GAMENAME + std::string("/");
#ifdef PC
    graphLib.set_window_icon();
#endif
	fio.read_game(game_data);

    gameControl.get_drop_item_ids();
	soundManager.init_audio_system();

    // define SAVEPATH
#ifdef PLAYSTATION2
        PS2_load_MC();
        //SAVEPATH = "mc0:Rockbot/";
        SAVEPATH = FILEPATH;

        /*
        if (fioMkdir(SAVEPATH.c_str()) < 0) {
            /// @TODO - check if directory exists
        }
        */

#endif


    graphLib.preload();

#ifdef PLAYSTATION2
    fio.load_config(game_config);
    PS2_create_save_icons();
#endif

    draw_lib.preload();


    gameControl.currentStage = INTRO_STAGE;

    // === DEBUG === //
    //game_config.volume_sfx = 128;
    //game_config.volume_music = 128;
    //GAME_FLAGS[FLAG_QUICKLOAD] = true;
    //GAME_FLAGS[FLAG_INVENCIBLE] = true;
    // === DEBUG === //


    input.clean();
    input.p1_input[BTN_START] = 0;
    timer.delay(200);

    input.clean();

    if (current_stage != -1) {
        gameControl.set_current_stage(current_stage);
    }

    // INIT GAME
    if (GAME_FLAGS[FLAG_QUICKLOAD] == false) {
        if (gameControl.show_game_intro() == false) {
            return 0;
        }
    } else {
        gameControl.quick_load_game();
    }

    fflush(stdout);


    while (run_game) {
        timer.start_ticker();
        gameControl.show_game(true, true);
        draw_lib.update_screen();
        if (input.p1_input[BTN_QUIT] == 1) {
            std::fflush(stdout);
            leave_game = true;
        }

    }


    SDL_Quit();
	
	return 1;
}


#ifdef ANDROID
extern "C" {
    JNIEXPORT void JNICALL Java_net_upperland_rockdroid_DemoRenderer_nativeInit(JNIEnv * env, jobject obj);
};

JNIEXPORT void JNICALL Java_net_upperland_rockdroid_DemoRenderer_nativeInit(JNIEnv * env, jobject obj)
{
    char cwd[1024];
    /*
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", cwd);
    } else {
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "cwd is NULL");
    }
    */
    char *argv[1];
    argv[0] = "./";
    activity_ref = obj;
    main(1, argv);
    /*
    } catch (std::invalid_argument e) {
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "Exception[invalid_argument]");
        android_game_services android_services;
        android_services.crash_handler(e.what());
    } catch (std::runtime_error e) {
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT###", "Exception[runtime_error]");
        android_game_services android_services;
        android_services.crash_handler(e.what());
    }
    */
}



#endif

