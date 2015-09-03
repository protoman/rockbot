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
    /**
     * @brief
     *
     */
    ~game();
    /**
     * @brief
     *
     */
    void initGame();
    /**
     * @brief
     *
     * @return bool
     */
    bool showIntro();
    /**
     * @brief
     *
     */
    void quick_load_game();


    void update_stage_scrolling();
    void showGame(bool can_characters_move=true);
    /**
     * @brief
     *
     * @param st_position
     * @return int
     */
    int getMapPointLock(struct st_position);

    /**
     * @brief
     *
     * @return st_position
     */
    st_position checkScrolling();
    /**
     * @brief
     *
     */
    void fps_count();
    /**
     * @brief
     *
     * @param direction
     * @param is_door
     * @param tileX
     * @param tileY
     */
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


    void leave_game();

    /**
     * @brief
     *
     */
    void game_over();
    /**
     * @brief
     *
     */
    void show_ending(st_position boss_pos);
    /**
     * @brief
     *
     * @param centerX
     * @param centerY
     * @param show_players
     */
    void draw_explosion(short int centerX, short int centerY, bool show_players);
    /**
     * @brief
     *
     * @param centerX
     * @param centerY
     */
    void draw_implosion(short int centerX, short int centerY);
	// PLAYER methods
    /**
     * @brief
     *
     * @param player_n
     */
    void show_player(short int player_n);
    /**
     * @brief
     *
     * @param pos
     * @param player_n
     */
    void set_player_position(st_position pos, short int player_n);

    /**
     * @brief
     *
     * @param xinc
     * @param yinc
     * @param player_n
     */
    void change_player_position(short xinc, short yinc, short player_n);
    /**
     * @brief
     *
     * @param anim_type
     * @param player_n
     */
    void set_player_anim_type(ANIM_TYPE anim_type, short int player_n);
    /**
     * @brief
     *
     * @param player_n
     * @return st_position
     */
    st_position get_player_position(short int player_n);
    /**
     * @brief
     *
     * @param player_n
     * @return st_size
     */
    st_size get_player_size(short int player_n);
    /**
     * @brief
     *
     * @param direction
     * @param player_n
     */
    void set_player_direction(short int direction, short int player_n);
    /**
     * @brief
     *
     * @param player_n
     */
    void player_fall(short int player_n);
    /**
     * @brief
     *
     * @param show_dialog
     */
    void map_present_boss(bool show_dialog);

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
    bool must_show_boss_hp() const;
    /**
     * @brief
     *
     */
    void fill_boss_hp_bar() const;


    void fill_player_weapon(short weapon_n);

    /**
     * @brief
     *
     */
    void reset_stage_maps() const;
    /**
     * @brief
     *
     */
    void remove_all_projectiles();

    void reset_beam_objects();


    void remove_players_slide();
    /**
     * @brief
     *
     */
    void show_map() const;
    /**
     * @brief
     *
     * @param int
     */
    void set_current_map(int);

    /**
     * @brief
     *
     * @param n
     * @return classPlayer
     */
    classPlayer* get_player(int n);
    /**
     * @brief
     *
     * @return st_position
     */
    st_position get_current_stage_scroll() const;
    /**
     * @brief
     *
     */
    void reset_scroll() const;
    /**
     * @brief
     *
     * @param type
     * @return short
     */
    short get_drop_item_id(short type) const;
    /**
     * @brief
     *
     */
    void get_drop_item_ids();
    /**
     * @brief
     *
     * @param finished_stage
     */
    bool show_config(short finished_stage);


    void unload_stage();

    void show_savegame_error();

    void showGotArmorDialog(e_ARMOR_PIECES armor_type);

    void object_teleport_boss(st_position dest_pos, int dest_map, int teleporter_id);

    void remove_current_teleporter_from_list(); // used when player dies



private:
    /**
     * @brief
     *
     * @param test_player
     * @return bool
     */
    bool test_teleport(classPlayer *test_player);

    /**
     * @brief
     *
     */
    void start_stage();


    void show_ready() const;

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
    void transitionScreen(short int type, short int map_n, short int adjust_x, classPlayer *pObj);
    /**
     * @brief
     *
     * @return short
     */
    short int get_current_map() const;
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

    bool is_player_on_teleporter() const;

    void finish_player_teleporter();

    void show_stage(int wait_time, bool move_npcs);

    bool subboss_alive_on_left(short tileX);



public:
    int currentStage; /**< TODO */
    bool is_showing_boss_intro; /**< TODO */


private:
    stage* loaded_stage; /**< TODO */
    std::vector<classPlayer> players; /**< TODO */
    scenesLib scenes; /**< TODO */
    unsigned int fps_timer; /**< TODO */
    int fps_counter; /**< TODO */
    std::stringstream fps_msg; /**< TODO */
    char _fps_buffer[128]; /**< TODO */
    struct CURRENT_FILE_FORMAT::st_game_config config; /**< TODO */
    st_position selected_stage; /**< TODO */
    class_config config_manager; /**< TODO */
    dialogs game_dialogs; /**< TODO */
	// framerate timers
    float _frame_duration; /**< TODO */
    std::map<short, bool> _last_stage_used_teleporters; // list of used teleportes (they do not work anymore after added to this list) /**< TODO */
    used_teleporter _player_teleporter;
    bool _show_boss_hp; // after set to true, will keep showing the boss HP bar on screen right side /**< TODO */

    short _drop_item_list[DROP_ITEM_COUNT];
    bool invencible_old_value; // used to store flag value in order we don't loose it when setting to true due to temporary "got weapon" invencibility /**< TODO */
    bool _dark_mode;                    // on dark mode we only show animation and projectiles

#ifdef PSP
    psp_ram _ram_counter;
    //std::cout << "unload_stage::RAM::BF='" << ram_counter.ramAvailable() << "'" << std::endl;
#endif

};

#endif // GAME_H
