#ifndef CLASSMAP_H
#define CLASSMAP_H


#include "defines.h"
#include "file/format/st_common.h"
#include "graphicslib.h"
#include "character/classnpc.h"
#include "objects/object.h"
#include "graphic/animation.h"
#include <list>
#include <vector>

#define EXPLOSION_ANIMATION_TIME 50

class classPlayer; // forward declaration

class character;

/**
 * @brief
 *
 */
struct object_colision {
    int _block;
    object* _object;
	object_colision(int set_block, object* set_object) {
		_block = set_block;
		_object = set_object;
	}
    object_colision() {
        _block = 0;
        _object = NULL;
    }
};

struct water_bubble {
    st_position pos;
    float x_adjust;
    int x_adjust_direction;
    unsigned int timer;
};


struct st_level3_tile {
    st_position tileset_pos;
    st_position map_position;
    st_level3_tile(st_position set_tileset_pos, st_position set_map_position)
    {
        tileset_pos = set_tileset_pos;
        map_position = set_map_position;
    }
    st_level3_tile()
    {
        tileset_pos = st_position(0, 0);
        map_position = st_position(0, 0);
    }

};


/**
 * @brief
 *
 */
class classMap
{
public:
	classMap();
    ~classMap();
    static bool deleteAll( classMap* theElement );

    void setStageNumber(int);

    short get_number() const;

    void setMapNumber(int);

    void loadMap();

    void drawMap();

    void showMap();

    void showAbove(int scroll_y=0, int temp_scroll_x = -99999);

    bool is_point_solid(st_position pos) const;

    int getMapPointLock(st_position pos) const;

    st_position_int8 get_map_point_tile1(st_position pos);

    bool get_map_point_wall_lock(int x) const;

    void changeScrolling(st_position pos, bool check_lock=true);

    st_position getMapScrolling() const;

    st_position get_last_scrolled() const;

    void reset_scrolled();

    void load_map_objects();

    int colision_rect_player_obj(character* playerObj, object* temp_obj, const short int x_inc, const short int y_inc, short int reduce_x, short int reduce_y, const short obj_xinc, const short obj_yinc);

    void colision_player_object(character*, const short int, const short int, short int, short int);

    object_colision get_obj_colision();

    short int colision_player_npcs(character*, const short int, const short int, short int, short int);

    void colision_player_special_attack(character*, const short int, const short int, short int, short int);

    classnpc* find_nearest_npc(st_position pos);

    void clean_map_npcs_projectiles();

    void reset_beam_objects();

    graphicsLib_gSurface* get_map_surface();

    void set_scrolling(st_position pos);

    void reset_scrolling();

    void move_map(const short int move_x, const short int move_y);

    void clean_map();

    void reset_map();

    void redraw_boss_door(bool is_close, int nTiles, int tileX, int tileY, short player_number);

    void add_animation(ANIMATION_TYPES pos_type, graphicsLib_gSurface* surface, const st_float_position &pos, st_position adjust_pos, unsigned int frame_time, unsigned int repeat_times, int direction, st_size framesize);

    void clear_animations(); // remove all animations from map

    void set_player_list(std::vector<classPlayer*> player_list);

    classnpc *spawn_map_npc(short int npc_id, st_position npc_pos, short direction, bool player_friend, bool progressive_span);

    void move_npcs();

    void show_npcs();

    void move_objects(bool paused);

    std::vector<object*> check_collision_with_objects(st_rectangle collision_area);

    void show_objects(int adjust_y=0);

    bool boss_hit_ground();

    void reset_map_npcs();

    void draw_dynamic_backgrounds_into_surface(graphicsLib_gSurface &surface, int x_adjust, int y_adjust);

    void add_object(object obj);

    int get_first_lock_on_left(int x_pos) const;

    int get_first_lock_on_right(int x_pos) const;

    void drop_item(st_position pos);

    void set_bg1_scroll(int scrollx);

    void set_bg2_scroll(int scrollx);

    int get_bg1_scroll() const;

    int get_bg2_scroll() const;

    void reset_objects_timers();

    void reset_objects(); // restore objects to their original position

    void print_objects_number();

    void add_bubble_animation(st_position pos);

    bool have_player_object();

    bool subboss_alive_on_left(short tileX);

    void finish_object_teleporter(int number);

    void activate_final_boss_teleporter();

private:
    void load_map_npcs();

    void draw_dynamic_backgrounds();

    bool value_in_range(int value, int min, int max) const;

    void create_dynamic_background_surfaces();

    void create_dynamic_background_surface(graphicsLib_gSurface& dest_surface, graphicsLib_gSurface& image_surface) const;

public:
    std::vector<classnpc*> _npc_list; // vector npcs
    std::vector<classPlayer*> _player_list; // vector players
    std::vector<animation> animation_list;
	// vector teleporters
	// vector objects
    int number;

private:
    int stage_number;
    graphicsLib_gSurface mapSurface;
    struct st_position scroll;
    st_position scrolled;						// stores the value the map scrolled in this cycle. used for character movement control (it should move taking the scroll in account)
    std::vector<object> object_list;
    bool wall_scroll_lock[MAP_W];
    float bg1_scroll;
    float bg2_scroll;
    graphicsLib_gSurface bg1_surface;
    graphicsLib_gSurface bg2_surface;
    short _platform_leave_counter;
    water_bubble _water_bubble;
    st_rectangle _3rd_level_ignore_area;
    object_colision _obj_colision;
    std::vector<st_level3_tile> _level3_tiles;
    bool _break_npc_loop;                       // used to prevent looping through the npc list after adding a new one (needed because using vector instead of list, because of old-Dingux crashes)
};



#endif // CLASSMAP_H
