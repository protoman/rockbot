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


struct object_collision {
    int _block;
    object* _object;
	object_collision(int set_block, object* set_object) {
		_block = set_block;
		_object = set_object;
	}
    object_collision() {
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

struct anim_tile_desc {
    int anim_tile_id;
    int dest_x;
    int dest_y;

    anim_tile_desc(int id, st_position pos) {
        anim_tile_id = id;
        dest_x = pos.x;
        dest_y = pos.y;
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

    void showMap();

    void draw_map_tiles();

    void draw_animated_tiles(graphicsLib_gSurface &surface);

    void init_animated_tiles();

    void showAbove(int scroll_y=0, int temp_scroll_x = -99999);

    bool is_point_solid(st_position pos) const;

    int getMapPointLock(st_position pos) const;

    st_position_int8 get_map_point_tile1(st_position pos);

    bool get_map_point_wall_lock(int x) const;

    void changeScrolling(st_float_position pos, bool check_lock=true);

    st_float_position getMapScrolling() const;

    st_float_position* get_map_scrolling_ref();

    st_float_position get_last_scrolled() const;

    void reset_scrolled();

    void load_map_objects();

    int collision_rect_player_obj(st_rectangle player_rect, object* temp_obj, const short int x_inc, const short int y_inc, const short obj_xinc, const short obj_yinc);

    bool is_obj_ignored_by_enemies(Uint8 obj_type);                 // returns true if object if of type that can be got like energy

    void collision_char_object(character*, const float, const short int);

    object_collision get_obj_collision();

    classnpc* collision_player_npcs(character*, const short int, const short int);

    void collision_player_special_attack(character*, const short int, const short int, short int, short int);

    classnpc* find_nearest_npc(st_position pos);

    classnpc* find_nearest_npc_on_direction(st_position pos, int direction);

    void clean_map_npcs_projectiles();

    void reset_beam_objects();

    void get_map_area_surface(graphicsLib_gSurface &mapSurface);

    void set_scrolling(st_float_position pos);

    void reset_scrolling();

    void move_map(const short int move_x, const short int move_y);

    void reset_map();

    void redraw_boss_door(bool is_close, int nTiles, int tileX, int tileY, short player_number);

    void add_animation(ANIMATION_TYPES pos_type, graphicsLib_gSurface* surface, const st_float_position &pos, st_position adjust_pos, unsigned int frame_time, unsigned int repeat_times, int direction, st_size framesize);

    void add_animation(animation anim);

    void clear_animations(); // remove all animations from map

    void set_player(classPlayer* player_ref);

    classnpc *spawn_map_npc(short int npc_id, st_position npc_pos, short direction, bool player_friend, bool progressive_span);

    void move_npcs();

    void show_npcs();

    void move_objects(bool paused);

    std::vector<object*> check_collision_with_objects(st_rectangle collision_area);

    void show_objects(int adjust_y=0, int adjust_x=0);

    bool boss_hit_ground();

    void reset_map_npcs();

    void draw_dynamic_backgrounds_into_surface(graphicsLib_gSurface &surface);

    void add_object(object obj);

    int get_first_lock_on_left(int x_pos) const;

    int get_first_lock_on_right(int x_pos) const;

    int get_first_lock_on_bottom(int x_pos);

    void drop_item(int i);

    void set_bg_scroll(int scrollx);

    int get_bg_scroll() const;

    void reset_objects_timers();

    void reset_objects(); // restore objects to their original position

    void print_objects_number();

    void add_bubble_animation(st_position pos);

    bool have_player_object();

    bool subboss_alive_on_left(short tileX);

    void finish_object_teleporter(int number);

    void activate_final_boss_teleporter();

    Uint8 get_map_gfx();

    Uint8 get_map_gfx_mode();

    st_float_position get_bg_scroll();
    void set_bg_scroll(st_float_position pos);

    st_rectangle get_player_hitbox();

private:
    void load_map_npcs();

    void draw_dynamic_backgrounds();

    void draw_foreground_layer(int scroll_x, int scroll_y);

    void adjust_dynamic_background_position();

    void adjust_foreground_position();

    bool value_in_range(int value, int min, int max) const;

    void create_dynamic_background_surfaces();

    graphicsLib_gSurface* get_dynamic_bg();

    graphicsLib_gSurface* get_dynamic_foreground();


public:
    std::vector<classnpc> _npc_list; // vector npcs
    classPlayer* _player_ref; // vector players
    std::vector<animation> animation_list;
	// vector teleporters
	// vector objects
    int number;

private:
    int stage_number;
    struct st_float_position scroll;
    st_float_position scrolled;						// stores the value the map scrolled in this cycle. used for character movement control (it should move taking the scroll in account)
    bool wall_scroll_lock[MAP_W];
    st_float_position bg_scroll;
    st_float_position fg_layer_scroll;
    short _platform_leave_counter;
    water_bubble _water_bubble;
    st_rectangle _3rd_level_ignore_area;
    object_collision _obj_collision;
    std::vector<st_level3_tile> _level3_tiles;
    std::vector<object> object_list;
    // DRAW MEMBERS //
    int _show_map_pos_x;                            // this is used to compare the position that the map was drawn last time to the current scrolling to check if map needs to be redrawn
    graphicsLib_gSurface map_screen;                // use to avoid having to draw the tilesets each time we update screen
    std::vector<anim_tile_desc> anim_tile_list;     // list of animated tiles, so we don't need to loop through all tiles when drawing only the animated ones
};



#endif // CLASSMAP_H
