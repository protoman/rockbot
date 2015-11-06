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
#endif

#ifdef OSX
#include <mach-o/dyld.h>
#endif

#ifdef DREAMCAST
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
#define DEBUG_SHOW_FPS 1
//#define PS2LOADFROMFIXEDPOINT 1
//#define DISABLESOUND 1
#define PS2LINK 1
#define DEBUG_OUTPUT 1 // will output all DEBUG_COUT messages, comments this out to disable all console output messages

// GLOBAL/EXTERN VARIABLES
std::string GAMEPATH; // path without DATA/GAMES
std::string FILEPATH; // path including game-data dir
std::string SAVEPATH;
std::string GAMENAME; // the gamename, part of path
SDL_Event event;
bool have_save = false;

#include "defines.h"
#include "graphicslib.h"
#include "graphic/draw.h"
#include "inputlib.h"
#include "timerlib.h"
#include "soundlib.h"
#include "game.h"

#include "aux_tools/stringutils.h"

#define MAXPATHLEN 256

#if defined(LINUX) || defined(OSX)
    #include <errno.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <sys/param.h>
#elif defined(PLAYSTATION2) || defined(DINGUX) || defined(PSP) || defined(ANDROID) || defined(OPEN_PANDORA) || defined(WII)
    #include <unistd.h>
#elif defined(WIN32)
    #include <direct.h>
    #undef main // to build on win32
#endif

#if defined(DINGUX)
	std::string EXEC_NAME("rockbot.dge");
#elif defined(WIN32)
	std::string EXEC_NAME("rockbot.exe");
#elif defined(PLAYSTATION2)
	std::string EXEC_NAME("rockbot.elf");
#elif defined(PSP)
	std::string EXEC_NAME("EBOOT.PBP");
#elif defined(ANDROID)
    std::string EXEC_NAME("");
#else
    std::string EXEC_NAME("rockbot");
#endif

// INTERNAL GLOBALS
timerLib timer;
inputLib input;
soundLib soundManager;
graphicsLib graphLib;
draw draw_lib;
game gameControl;
CURRENT_FILE_FORMAT::st_save game_save;
struct CURRENT_FILE_FORMAT::st_game_config game_config;
bool GAME_FLAGS[FLAG_COUNT];

int default_keys_codes[BTN_COUNT]; // number indicator for the keyboard-keys
int default_button_codes[BTN_COUNT]; // number indicator for the keyboard-keys

bool leave_game = false;


#include "file/file_io.h"
CURRENT_FILE_FORMAT::file_io fio;
CURRENT_FILE_FORMAT::file_game game_data;
CURRENT_FILE_FORMAT::file_stage stage_data;
CURRENT_FILE_FORMAT::file_map map_data[FS_STAGE_MAX_MAPS];
std::vector<CURRENT_FILE_FORMAT::st_anim_map_tile> anim_tiles;

#include "defines.h"

FREEZE_EFFECT_TYPES freeze_weapon_effect = FREEZE_EFFECT_NONE;
int freeze_weapon_id = -1;

struct CURRENT_FILE_FORMAT::st_checkpoint checkpoint;

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
        std::string filename = FILEPATH + "images/icon.sys";
        PS2_copy_to_memorycard(filename, "icon.sys");
        filename = FILEPATH + "images/rockbot_ps2_icon.icn";
        PS2_copy_to_memorycard(filename, "rockbot_ps2_icon.icn");
    } else {
        fclose(fp);
    }
}



#endif


#ifdef PSP
PSP_MODULE_INFO("Rockbot", PSP_MODULE_USER, 1, 0);
//PSP_HEAP_SIZE_KB(-1024);
PSP_HEAP_SIZE_MAX();


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
    char* res = getcwd(buffer, MAXPATHLEN);
    UNUSED(res);
    if(buffer != NULL){
        FILEPATH = std::string(buffer);
    }
    FILEPATH += "/";
    delete[] buffer;
#endif

    #if defined(PLAYSTATION2) && defined(PS2LOADFROMFIXEDPOINT) // DEBUG
        FILEPATH = "mass:/PS2/Rockbot/";
    #elif WII
        FILEPATH = "sd:/apps/Rockbot/";
        printf("MAIN #D\n");
    #endif
    std::cout << "get_filepath - FILEPATH:" << FILEPATH << std::endl;
	#ifdef DREAMCAST
		FILEPATH = "/cd/";
	#endif

    GAMEPATH = FILEPATH;
}


/*
void execute_memory_test() {
#ifdef PSP
    psp_ram _ram_counter;
#endif

    int stage_number = 1;
    int map_id = 0;
    int npc_id = 0;
    int object_id = 0;

    fio.read_stage(stage_data, stage_number);


#ifdef PSP
    std::cout << "MEMTEST[NPC]::BEFORE='" << _ram_counter.ramAvailable() << "'" << std::endl;
#endif
    classnpc* new_npc = new classnpc(stage_number, map_id, stage_data.maps[map_id].map_npcs[npc_id].id_npc, npc_id);
    new_npc->clean_character_graphics_list();
    delete new_npc;
#ifdef PSP
    std::cout << "MEMTEST[NPC]::AFTER='" << _ram_counter.ramAvailable() << "'" << std::endl;
#endif

#ifdef PSP
    std::cout << "MEMTEST[OBJECT]::BEFORE='" << _ram_counter.ramAvailable() << "'" << std::endl;
#endif
    object* new_obj = new object(object_id, NULL, st_position(0, 0));
    new_obj->remove_graphic();
    delete new_obj;
#ifdef PSP
    std::cout << "MEMTEST[OBJECT]::AFTER='" << _ram_counter.ramAvailable() << "'" << std::endl;
#endif


#ifdef PSP
    std::cout << "MEMTEST[MAP]::BEFORE='" << _ram_counter.ramAvailable() << "'" << std::endl;
#endif
    classMap* new_map = new classMap();
    new_map->setMapNumber(map_id);
    new_map->setStageNumber(stage_number);
    new_map->loadMap();
    delete new_map;
#ifdef PSP
    std::cout << "MEMTEST[MAP]::AFTER='" << _ram_counter.ramAvailable() << "'" << std::endl;
#endif

    std::fflush(stdout);
    input.waitTime(200);
    std::fflush(stdout);
    std::cout << "MEMTEST[END]" << std::endl;
    input.waitTime(200);
}
*/

#ifdef WII
extern "C" int main(int argc, char *argv[])
#else

#ifdef DREAMCAST
	KOS_INIT_FLAGS(INIT_DEFAULT);
#endif

int main(int argc, char *argv[])
#endif
{


#ifdef PSP
    SetupCallbacks();
    //scePowerSetClockFrequency(333, 333, 166);
#endif

    for (int i=0; i<FLAG_COUNT; i++) {
		GAME_FLAGS[i] = false;
	}
	UNUSED(argc);

    string argvString = "";
#ifndef WII
    argvString = string(argv[0]);
#else
    if (!fatInitDefault()) {
        printf("fatInitDefault ERROR #1");
        std::fflush(stdout);
        timer.delay(500);
        exit(-1);
    }
#endif


    get_filepath();
    // fallback in case getcwd returns null
    if (FILEPATH.size() == 0) {
        std::cout << "Could not read path, fallback to using argv" << std::endl;
        FILEPATH = argvString.substr(0, argvString.size()-EXEC_NAME.size());
    }
    std::cout << "main - argvString: '" << argvString << "', FILEPATH: '" << FILEPATH << "'" << std::endl; std::fflush(stdout);



#ifdef PLAYSTATION2
    std::cout << "PS2.DEBUG #1" << std::endl; std::fflush(stdout);

    #ifndef PS2LINK
    SifIopReset(NULL, 0); // clean previous loading of irx by apps like ulaunchElf. Comment this line to get cout on ps2link
    #endif

    printf("DEBUG.PS2 #1.1\n");

	/* SP193: Being creative (Do something while waiting for the slow IOP to be reset). =D */
	int main_id = GetThreadId();
    ChangeThreadPriority(main_id, 72);
    std::cout << "PS2.DEBUG #1.1" << std::endl; std::fflush(stdout);
    printf("DEBUG.PS2 #1.2\n");

    #ifndef PS2LINK
    while(SifIopSync()) {
        std::cout << "PS2.SifIopSync()" << std::endl;
    }
    #endif
	/* Initialize and connect to all SIF services on the IOP. */
	SifInitRpc(0);
	SifInitIopHeap();
	SifLoadFileInit();
	fioInit();
    printf("DEBUG.PS2 #1.3\n");

	/* Apply the SBV LMB patch to allow modules to be loaded from a buffer in EE RAM. */
	sbv_patch_enable_lmb();

    // --- DEBUG --- //
    //FILEPATH = "cdfs:/";
    // --- DEBUG --- //

    std::cout << "PS2.DEBUG #2" << std::endl; std::fflush(stdout);

    if (FILEPATH.find("mass:") != std::string::npos) {
        printf("DEBUG.PS2 #1.4\n");
        std::cout << "PS2.DEBUG Load USB" << std::endl; std::fflush(stdout);
        PS2_load_USB();
    }

    if (FILEPATH.find("cdfs") != std::string::npos || FILEPATH.find("cdrom") != std::string::npos) {
        printf("DEBUG.PS2 #1.5\n");
        std::cout << "PS2.DEBUG Load CDROM" << std::endl; std::fflush(stdout);
        FILEPATH = "cdfs:";
        PS2_load_CDROM();
    }
    printf("DEBUG.PS2 #2\n");




    std::cout << "PS2.DEBUG #3" << std::endl; std::fflush(stdout);
#endif




	// check command-line paramethers
	if (argc > 1) {
		for (int i=1; i<argc; i++) {
			std::string temp_argv(argv[i]);
			if (temp_argv == "--fullscreen") {

			} else if (temp_argv == "--quickload") {
				GAME_FLAGS[FLAG_QUICKLOAD] = true;
			} else if (temp_argv == "--invencible") { // player have infinite HP
				GAME_FLAGS[FLAG_INVENCIBLE] = true;
			} else if (temp_argv == "--allweapons") { // player have all weapons available even if
				GAME_FLAGS[FLAG_ALLWEAPONS] = true;
			} else if (temp_argv == "--infinitejump") { // player can jump again and again
				GAME_FLAGS[FLAG_INFINITE_JUMP] = true;
            } else if (temp_argv == "--rockbot") {
                GAME_FLAGS[FLAG_PLAYER_ROCKBOT] = true;
            } else if (temp_argv == "--betabot") {
                GAME_FLAGS[FLAG_PLAYER_BETABOT] = true;
            } else if (temp_argv == "--candybot") {
                GAME_FLAGS[FLAG_PLAYER_CANDYBOT] = true;
            } else if (temp_argv == "--kittybot") {
                GAME_FLAGS[FLAG_PLAYER_KITTYBOT] = true;
            }
		}
	}

    /// DEBUG ///
    //GAME_FLAGS[FLAG_QUICKLOAD] = true;

    // PS2 version have to load config AFTER SDL_Init due to SDK issues
    #ifdef LINUX
        SAVEPATH = std::string(getenv("HOME")) + "/.rockbot/";
        mkdir(SAVEPATH.c_str(), 0777);
        //std::cout << "SAVEPATH: " << SAVEPATH << ", mkdir-res: " << res << ", errno: " << errno << std::endl;
    #elif WIN32
        SAVEPATH =  std::string(getenv("APPDATA")) + "/rockbot";
        std::cout << "SAVEPATH: " << SAVEPATH << std::endl;
        _mkdir(SAVEPATH.c_str());
    #elif DREAMCAST
        SAVEPATH = "/vmu/a1";
    #else
        SAVEPATH = GAMEPATH;
    #endif


    std::cout << "SAVEPATH: " << SAVEPATH << std::endl;

#ifndef PLAYSTATION2
    fio.load_config(game_config);
#endif

    // INIT GRAPHICS
    if (graphLib.initGraphics() != true) {
        std::cout << "ERROR intializing graphic mode." << std::endl;
        return -1;
    }


    GAMENAME = gameControl.select_game_screen();
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

    std::cout << "GAMENAME: " << GAMENAME << std::endl;

	fio.read_game(game_data);
    anim_tiles = fio.read_anim_tiles();


    //GAME_FLAGS[FLAG_INFINITE_HP] = true; // DEBUG


    gameControl.get_drop_item_ids();
	soundManager.init_audio_system();




    // define SAVEPATH
    #ifdef PLAYSTATION2
        PS2_load_MC();
        SAVEPATH = "mc0:Rockbot/";

        if (fioMkdir(SAVEPATH.c_str()) < 0) {
            std::cout << "main - warning: could not create '" << SAVEPATH << "' folder" << std::endl; std::fflush(stdout);
            /// @TODO - check if directory exists
        } else {
            std::cout << "Folder '" << SAVEPATH << "' created" << std::endl; std::fflush(stdout);
        }

    #endif
    have_save = fio.save_exists();

    #ifndef DEBUG_OUTPUT // redirect output to null
        std::string cout_file = "/dev/null";
        std::ofstream out(cout_file.c_str());
        std::cout.rdbuf(out.rdbuf());
    #else
        // --- REDIRECT STDOUT TO A FILE --- //
        #if defined(PSP) || defined(WII) || defined(ANDROID) || defined(DINGUX) || defined(PLAYSTATION2)
            //std::string cout_file = SAVEPATH + "/stdout.txt";
            std::string cout_file = GAMEPATH + "/stdout.txt";
            std::streambuf *coutbuf = std::cout.rdbuf();
            std::ofstream out(cout_file.c_str());
            std::cout.rdbuf(out.rdbuf()); //redirect std::cout to out.txt!
        #endif
    #endif


    graphLib.preload();
#ifdef PLAYSTATION2
    fio.load_config(game_config);
    PS2_create_save_icons();
#endif
    draw_lib.preload();

    gameControl.currentStage = INTRO_STAGE;





	// INIT GAME
	if (GAME_FLAGS[FLAG_QUICKLOAD] == false) {
		if (gameControl.showIntro() == false) {
            std::cout << "ERROR SHOWING INTRO" << std::endl;
			return 0;
		}
	} else {
        gameControl.quick_load_game();
        //ending end_obj;
        //end_obj.start();
        //return 1;
    }

	input.clean();
	input.p1_input[BTN_START] = 0;
	input.waitTime(200);
	input.clean();

    bool run_game = true;

    while (run_game) {
        #if !defined(DINGUX)
            timer.start_ticker();
        #endif


		#ifdef PLAYSTATION2
			RotateThreadReadyQueue(_MIXER_THREAD_PRIORITY);
        #endif


		gameControl.showGame();
#ifdef DEBUG_SHOW_FPS
        gameControl.fps_count();
#endif
        draw_lib.update_screen();
        if (input.p1_input[BTN_QUIT] == 1) {
            std::fflush(stdout);
            leave_game = true;
        }
        unsigned int now_ticks = timer.get_ticks();
        if (now_ticks < (1000 / FRAMES_PER_SECOND)) {
            timer.delay((1000 / FRAMES_PER_SECOND) - now_ticks);
        }

    }
	/// @TODO: sdl quit sub-systems


	
#ifdef PSP
    sceKernelExitGame();
    return 0;
#else
    SDL_Quit();
#endif
	
	return 1;
}


#ifdef ANDROID
extern "C" {
    JNIEXPORT void JNICALL Java_net_upperland_rockbot_DemoRenderer_nativeInit(JNIEnv * env, jobject obj);
};

JNIEXPORT void JNICALL Java_net_upperland_rockbot_DemoRenderer_nativeInit(JNIEnv * env, jobject obj)
{
    char * argv[1];
    argv[0] = "./";
    main(1, argv);
}

#endif

