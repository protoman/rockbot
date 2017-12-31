#ifndef GAME_H
#define GAME_H

#include <map>
#include <vector>
#include <sstream>
#include <string>

class classPlayer;
class classnpc;


#include "character/classplayer.h"
#include "character/classnpc.h"
#include "sceneslib.h"
#include "stage.h"
#include "objects/object.h"
#include "class_config.h"
#include "scenes/dialogs.h"
#include "aux_tools/fps_control.h"


#ifdef PSP
#include "ports/psp/psp_ram.h"
#endif

/**
 * @brief
 *
 */
class game
{
public:
/**
 * @brief
 *
 */
    game();
    ~game();
    void initGame();
    /**
     * @brief
     *
     * @return bool
     */
    bool show_game_intro();

    void show_beta_version_warning();

    void show_free_version_warning();

    void show_notice();

    /**
     * @brief
     *
     */
    void quick_load_game();

    void set_player_direction(ANIM_DIRECTION dir);
    void show_player_at(int x, int y);


    void update_stage_scrolling();
    void show_game(bool can_characters_move, bool can_scroll_stage);
    /**
     * @brief
     *
     * @param st_position
     * @return int
     */
    Uint8 getMapPointLock(struct st_position);

    /**
     * @brief
     *
     * @return st_position
     */
    st_float_position checkScrolling();
    void horizontal_screen_move(short direction, bool is_door, short tileX, short tileY);
    /**
     * @brief
     *
     */
    void got_weapon();
    /**
     * @brief
     *
     */
    void leave_stage();

    void return_to_intro_screen();

    void game_pause();
    void game_unpause();

    /**
     * @brief
     *
     */
    void game_over();
    /**
     * @brief
     *
     */
    void show_ending();
    void show_demo_ending();


    void draw_explosion(short int centerX, short int centerY, bool show_players);
    void show_player();
    /**
     * @brief
     *
     * @param pos
     * @param player_n
     */
    void set_player_position(st_position pos);

    /**
     * @brief
     *
     * @param xinc
     * @param yinc
     * @param player_n
     */
    void change_player_position(short xinc, short yinc);
    /**
     * @brief
     *
     * @param anim_type
     * @param player_n
     */
    void set_player_anim_type(ANIM_TYPE anim_type);
    /**
     * @brief
     *
     * @param player_n
     * @return st_position
     */
    st_position get_player_position();
    /**
     * @brief
     *
     * @param player_n
     * @return st_size
     */
    st_size get_player_size();
    /**
     * @brief
     *
     * @param direction
     * @param player_n
     */
    void set_player_direction(Uint8 direction);
    void map_present_boss(bool show_dialog, bool is_static_boss);

    character* get_player();

    /**
     * @brief
     *
     * @return object
     */
    object* get_player_platform();
    /**
     * @brief
     *
     */
    void check_player_return_teleport(); // if player is inside a teleporter, killing a boss makes him teleport out
    /**
     * @brief
     *
     * @return bool
     */
    bool must_show_boss_hp();


    void reset_stage_maps();
    void remove_all_projectiles();

    void reset_beam_objects();

    void remove_temp_objects();


    void remove_players_slide();
    void show_map();
    void set_current_stage(int stage);
    void set_current_map(int);
    st_float_position get_current_stage_scroll();
    /**
     * @brief
     *
     */
    void reset_scroll();
    short get_drop_item_id(short type);
    void get_drop_item_ids();
    bool show_config(short finished_stage);
    void show_savegame_error();
    void showGotArmorDialog(e_ARMOR_PIECES armor_type);
    void object_teleport_boss(st_position dest_pos, Uint8 dest_map, Uint8 teleporter_id);
    void remove_current_teleporter_from_list(); // used when player dies
    void select_game_screen();
    std::string get_selected_game();
    bool is_free_version();
    classMap* get_current_map_obj();
    bool is_player_on_teleporter();
    short get_last_castle_stage();
    short get_current_save_slot();
    void set_current_save_slot(short n);
    void save_game();
    void set_show_fps_enabled(bool enabled);
    bool get_show_fps_enabled();

#ifdef ANDROID
    bool load_save_data_from_cloud();
#endif


private:
    void exit_game();
    bool test_teleport(classPlayer *test_player);
    bool check_player_is_on_teleport(classPlayer *test_player, int currentMap, int temp_x, int temp_y, int link_n, int transition_type, int &teleport_count, int &teleporter_dist, int &player_x, int &link_type);
    bool is_link_teleporter(int type);
    void start_stage();
    void set_player_position_teleport_in(int initial_pos_x);
    void show_player_teleport(int pos_x);


    void show_ready();

    /**
     * @brief
     *
     */
    void restart_stage();
	//void load_game_objects();

    /**
     * @brief
     *
     * @param type
     * @param map_n
     * @param adjust_x
     * @param pObj
     */
    void transition_screen(Uint8 type, Uint8 map_n, short int adjust_x, classPlayer *pObj);
    /**
     * @brief
     *
     * @return short
     */
    Uint8 get_current_map();
    /**
     * @brief
     *
     * @param char_obj
     * @param pos_x
     */
    void walk_character_to_screen_point_x(character* char_obj, short pos_x); // keeps walking (and jumping obstacles) until reaching a given point in screen (not in map, that should have its own function for that)
    /**
     * @brief
     *
     * @param set_teleport_n
     * @param set_player_pos
     */
    void set_player_teleporter(short set_teleport_n, st_position set_player_pos, bool is_object);


    void finish_player_teleporter();

    void show_stage(int wait_time, bool move_npcs);

    bool subboss_alive_on_left(short tileX);

    void show_mem_debug(int n);



public:
    Uint8 currentStage;
    bool is_showing_boss_intro;



private:
    stage loaded_stage;
    classPlayer player1;
    scenesLib scenes;
    std::stringstream fps_msg;
    struct CURRENT_FILE_FORMAT::st_game_config config;
    st_position selected_stage;
    class_config config_manager;
    dialogs game_dialogs;
	// framerate timers
    float _frame_duration;
    std::map<short, bool> _last_stage_used_teleporters; // list of used teleportes (they do not work anymore after added to this list)
    used_teleporter _player_teleporter;
    bool _show_boss_hp; // after set to true, will keep showing the boss HP bar on screen right side

    short _drop_item_list[DROP_ITEM_COUNT];
    bool invencible_old_value; // used to store flag value in order we don't loose it when setting to true due to temporary "got weapon" invencibility
    bool _dark_mode;                    // on dark mode we only show animation and projectiles

    std::string _selected_game;

    fps_control fps_manager;

    short current_save_slot;

    bool show_fps_enabled;


#ifdef PSP
    psp_ram _ram_counter;
    //std::cout << "unload_stage::RAM::BF='" << ram_counter.ramAvailable() << "'" << std::endl;
#endif

};

#endif // GAME_H
