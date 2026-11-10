#include "character.h"
#include "../game.h"
#include "../timerlib.h"

#include "shareddata.h"

extern game gameControl;
extern timerLib timer;

#include "../soundlib.h"
extern soundLib soundManager;

#include "../inputlib.h"
extern inputLib input;

#include "../game_mediator.h"

#ifdef ANDROID
#include <android/log.h>
#endif

#define STAIR_ANIMATION_WAIT_FRAMES 10
#define STAIRS_GRAB_TIMEOUT 200
#define SLIDE_Y_ADJUST 12


extern bool GAME_FLAGS[FLAG_COUNT];

extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::st_save game_save;

extern FREEZE_EFFECT_TYPES freeze_weapon_effect;
extern int freeze_weapon_id;

// initialize static member
static std::map<std::string, graphicsLib_gSurface> _character_frames_surface;

// init character with default values
// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
character::character() : hitPoints(1, 1), last_hit_time(0), is_player_type(false), _platform(NULL), hit_animation_timer(0), hit_moved_back_n(0), jump_button_released(true), attack_button_released(true), dead(false), charging_color_n(0), charging_color_timer(0), shield_type(0), _moving_platform_timer(0), position(), _number(0), _super_jump(false), _force_jump(false), _teleport_minimal_y(0), _is_falling(false), _dead_state(0), slide_type(0), _water_splash(false), _has_background(false), hit_duration(300), _is_boss(false), _is_stage_boss(false), is_ghost(false)
{
    state.set_animation_restarted(false);
    move_speed = 2.0 * SharedData::get_instance()->get_movement_multiplier();

    accel_speed_y = 1;
    gravity_y = 0.25;
	position.y = 0;
	position.x = 0;
    can_fly = false;
	attack_state = ATTACK_NOT;
	max_projectiles = 1;
    _debug_char_name = "PLAYER_0";
    _stairs_stopped_count = 0;
    _charged_shot_projectile_id = -1;
    _normal_shot_projectile_id = 0;
    state.set_on_last_frame(false);
    _simultaneous_shots = 1;
    _ignore_gravity = false;
    _always_move_ahead = false;
    _was_hit = false;
    _progressive_appear_pos = 0;
    last_execute_time = 0;
    _check_always_move_ahead = false;
    _dropped_from_stairs = false;
    character_max_jump_number = 1;
    _dash_button_released = true;
    _damage_modifier = 0;
    _hit_ground = false;
    _dashed_jump = false;
    _can_execute_airdash = true;
    _air_dashed = false;
    _player_must_reset_colors = false;
    hit_animation_count = 0;
    _attack_frame_n = -1;
    state.set_on_attack_frame(false);
    _stairs_falling_timer = 0;
    attack_button_pressed_timer = 0;
    attack_button_last_state = 0;
    must_show_dash_effect = false;
}


// init character with default values
// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
character::~character()
{
	clean_projectiles();
	/// @TODO free map _character_frames_surface
}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void character::char_update_real_position() {
    if (gameControl.get_current_map_obj() != NULL) {
        realPosition.x = position.x - (int)gameControl.get_current_map_obj()->getMapScrolling().x;
        realPosition.y = position.y - (int)gameControl.get_current_map_obj()->getMapScrolling().y;
    } else {
		realPosition.x = position.x;
		realPosition.y = position.y;
    }
}


st_float_position character::get_screen_position_from_point(st_float_position pos)
{
    st_float_position res_pos;
    if (gameControl.get_current_map_obj() != NULL) {
        res_pos.x = pos.x - (int)gameControl.get_current_map_obj()->getMapScrolling().x;
        res_pos.y = pos.y - (int)gameControl.get_current_map_obj()->getMapScrolling().y;
    } else {
        res_pos.x = pos.x;
        res_pos.y = pos.y;
    }
    return res_pos;
}


bool character::is_on_screen()
{
    if (gameControl.get_current_map_obj() == NULL) {
        return false;
    }

    st_float_position scroll = gameControl.get_current_map_obj()->getMapScrolling();
    float pos_x = position.x;
    //float pos_y = position.y; // TBD
    // width of the sprite (a typo here, "pos_x+width", made enemies far behind the screen count as on/near the screen:
    // they kept running their AI and shooting, and the wall check for off-screen shots did not apply to them)
    int frame_w = total_frame_size.width;

    if (!is_player()) {
        pos_x -= GameMediator::get_instance()->get_enemy(_number)->sprites_pos_bg.x;
        //pos_y -= GameMediator::get_instance()->get_enemy(_number)->sprites_pos_bg.y;
        if (_has_background) {
            frame_w = graphLib.character_graphics_background_list.find(name)->second.width;
        }
        //if (name == "BULLDOZER") std::cout << "CHAR::is_on_screen[" << name << "], pos_x[" << pos_x << "], scroll.x[" << scroll.x << "]" << std::endl;
    }

    // is on screen plus a bit more on both sides
    if (abs(pos_x+frame_w*2) >= scroll.x && abs(pos_x-frame_w*2) <= scroll.x+RES_W) {
        //if (name == "BIG FISH") std::cout << "CHAR::is_on_screen - TRUE #1" << std::endl;
        return true;
    }

    // regular enemies work only on a limited screen
    if (is_stage_boss() == false) {
        //if (name == "BIG FISH") std::cout << "CHAR::is_on_screen - FALSE #1" << std::endl;
        return false;
    }

    // is on left of the screen
    if (abs(pos_x) > scroll.x-RES_W/2 && abs(pos_x) < scroll.x) {
        // check wall-lock on the range
        int map_point_start = (scroll.x-RES_W/2)/TILESIZE;
        int map_point_end = scroll.x/TILESIZE;
        bool found_lock = false;
        for (int i=map_point_start; i<=map_point_end; i++) {
            if (gameControl.get_current_map_obj()->get_map_point_wall_lock(i) == true) {
                found_lock = true;
            }
        }
        if (found_lock == false) {
            //if (name == "BIG FISH") std::cout << "CHAR::is_on_screen - TRUE #2" << std::endl;
            return true;
        }
    }

    // is on right to the screen
    if (abs(pos_x) > scroll.x+RES_W && abs(pos_x) < scroll.x+RES_W*1.5) {
        int map_point_start = (scroll.x+RES_W)/TILESIZE;
        int map_point_end = (scroll.x*1.5)/TILESIZE;
        bool found_lock = false;
        for (int i=map_point_start; i<=map_point_end; i++) {
            if (gameControl.get_current_map_obj()->get_map_point_wall_lock(i) == true) {
                found_lock = true;
            }
        }
        if (found_lock == false) {
            //if (name == "BIG FISH") std::cout << "CHAR::is_on_screen - TRUE #2" << std::endl;
            return true;
        }
    }
    //if (name == "BIG FISH") std::cout << "CHAR::is_on_screen - FALSE #2" << std::endl;
    return false;
}


bool character::is_on_visible_screen()
{
    if (gameControl.get_current_map_obj() == NULL) { // used ins scenes
        return true;
    }

    st_float_position scroll = gameControl.get_current_map_obj()->getMapScrolling();
    // entre scroll.x e scroll.x+RES_W
    float pos_x = position.x;
    //float pos_y = position.y; // TBD
    // width of the sprite (a typo here, "pos_x+width", made enemies far behind the screen count as on/near the screen:
    // they kept running their AI and shooting, and the wall check for off-screen shots did not apply to them)
    int frame_w = total_frame_size.width;

    if (!is_player()) {
        pos_x -= GameMediator::get_instance()->get_enemy(_number)->sprites_pos_bg.x;
        //pos_y -= GameMediator::get_instance()->get_enemy(_number)->sprites_pos_bg.y;
        //if (name == "BIG FISH") std::cout << "CHAR::is_on_screen[" << name << "], pos_x[" << pos_x << "], pos_y[" << pos_y << "]" << std::endl;
        if (_has_background) {
            frame_w = graphLib.character_graphics_background_list.find(name)->second.width;
        }
    }

    if (abs(pos_x + frame_w) >= scroll.x && abs(pos_x) < scroll.x+RES_W) {
        return true;
    }
    return false;
}


bool character::is_entirely_on_screen()
{
    if (gameControl.get_current_map_obj() == NULL) { // used ins scenes
        return true;
    }
    st_float_position scroll = gameControl.get_current_map_obj()->getMapScrolling();
    float my_pos = abs((float)position.x+(float)frameSize.width);
    int limit_min = scroll.x+TILESIZE;
    int limit_max = scroll.x+RES_W;

    if (my_pos >= limit_min && my_pos <= limit_max) {
        return true;
    }
    return false;
}


bool character::is_invisible() const
{
	return state.invisible;
}


st_float_position *character::get_position_ref()
{
	return &position;
}



// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
st_float_position character::getPosition() const
{
	/// @TODO - this is crashing sometimes
	return position;
}


void character::set_position(struct st_position new_pos)
{
	position.x = new_pos.x;
	position.y = new_pos.y;
    char_update_real_position();
}


void character::inc_position(float inc_x, float inc_y)
{
    position.x += inc_x;
    position.y += inc_y;
}




// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void character::set_is_player(bool set_player)
{
	is_player_type = set_player;
}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
bool character::is_player() const
{
	return is_player_type;
}




// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
string character::get_name(void) const
{
	return name;
}



// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
st_size character::get_size() const
{
    return frameSize;
}


st_rectangle character::get_hitbox(int anim_type)
{
    float x = position.x;
    float y = position.y;
    float w = frameSize.width;
    float h = frameSize.height;

    if (anim_type == -1) {
        anim_type = state.animation_type();
    }

    // player hitbox is hardcoded
    if (is_player()) {
        x = position.x + GameMediator::get_instance()->player_list_v3_1[_number].sprite_hit_area.x;
        y = position.y + GameMediator::get_instance()->player_list_v3_1[_number].sprite_hit_area.y;
        w = GameMediator::get_instance()->player_list_v3_1[_number].sprite_hit_area.w;
        h = GameMediator::get_instance()->player_list_v3_1[_number].sprite_hit_area.h;
        if (anim_type == ANIM_TYPE_SLIDE && slide_type == 1) {
            y = position.y + GameMediator::get_instance()->player_list_v3_1[_number].sprite_hit_area.y + SLIDE_Y_ADJUST;
            h = GameMediator::get_instance()->player_list_v3_1[_number].sprite_hit_area.h - SLIDE_Y_ADJUST;
        }
        //std::cout << "CHAT:get_hitbox - pos.x[" << position.x << "], hit.x[" << x << "], adjust.x[" << GameMediator::get_instance()->player_list_v3_1[_number].sprite_hit_area.x << "]" << std::endl;
    } else {
        int anim_n = state.animation_state();

        // prevent getting size from a frame that does not have information, use Vulnerable-area or hitbox from STAND instead
        st_rectangle col_rect;
        if (GameMediator::get_instance()->get_enemy(_number)->sprites[anim_type][anim_n].used == true) {
            col_rect = GameMediator::get_instance()->get_enemy(_number)->sprites[anim_type][anim_n].collision_rect;
        } else if (GameMediator::get_instance()->get_enemy(_number)->sprites[ANIM_TYPE_STAND][0].used == true) {
            col_rect = GameMediator::get_instance()->get_enemy(_number)->sprites[ANIM_TYPE_STAND][0].collision_rect;
        } else {
            col_rect = st_rectangle(GameMediator::get_instance()->get_enemy(_number)->sprites_pos_bg.x,
                                    GameMediator::get_instance()->get_enemy(_number)->sprites_pos_bg.y,
                                    GameMediator::get_instance()->get_enemy(_number)->frame_size.width,
                                    GameMediator::get_instance()->get_enemy(_number)->frame_size.height);
        }

        if (!_has_background) {
            if (state.direction() == ANIM_DIRECTION_LEFT) {
                x = position.x + frameSize.width - col_rect.x - col_rect.w;
            } else {
                x += col_rect.x;
            }
            y += col_rect.y;
        } else {
            //if (name == "OCTOPUS") std::cout << "ADJUST-FOR-BG - x: " << (x - gameControl.get_current_map_obj()->getMapScrolling().x) << ", y: " << y << ", bg-x: " << GameMediator::get_instance()->get_enemy(_number)->sprites_pos_bg.x << ", bg-y: " << GameMediator::get_instance()->get_enemy(_number)->sprites_pos_bg.y << std::endl;
            if (vulnerable_area_box.w > frameSize.width) {
                x -= GameMediator::get_instance()->get_enemy(_number)->sprites_pos_bg.x;
                y -= GameMediator::get_instance()->get_enemy(_number)->sprites_pos_bg.y;
            }
        }
        w = col_rect.w;
        h = col_rect.h;
    }
    
    // if (!is_player()) std::cout << "NPC[" << name << "] - has-bg[" << _has_background << "], pos.x[" << position.x << "], hitbox[" << x << ", " << y << ", " << w << ", " << h << "]" << std::endl;
    return st_rectangle(x, y, w, h);
}


st_rectangle character::get_vulnerable_area(int anim_type)
{
    float x = position.x;
    float y = position.y;
    float w = frameSize.width;
    float h = frameSize.height;

    // TODO: fix position for NPC with background //

    if (vulnerable_area_box.w != 0 && vulnerable_area_box.h != 0) { // use vulnerable area
        y = position.y + vulnerable_area_box.y;
        if (have_background_graphics() == false) {
            // the area is defined for the sprite as drawn facing right: when facing left it is mirrored inside the frame
            // (same rule as get_hitbox). Before, facing right moved it by -x instead of +x, away from the boss's face.
            if (state.direction() == ANIM_DIRECTION_LEFT) {
                x = position.x + frameSize.width - vulnerable_area_box.x - vulnerable_area_box.w;
            } else {
                x = position.x + vulnerable_area_box.x;
            }
            //if (!is_player()) std::cout << "### HITAREA (custom vulnerable-area, no BG) - NPC[" << name << "] - x[" << x << "], y[" << y << "], graph.w[" << graphLib.character_graphics_background_list.find(name)->second.width << "]" << std::endl;
        } else {
            x = position.x;
            y = position.y;
            // if hitbox-area.w is smaller than the bg-width or sprite-width, we need to take it into account
            if (vulnerable_area_box.w > frameSize.width) {
                st_position bg_pos = st_position(position.x-background_pos.x, position.y-background_pos.y);
                if (state.direction() == ANIM_DIRECTION_RIGHT) {
                    int bg_pos_with_w = bg_pos.x + graphLib.character_graphics_background_list.find(name)->second.width;
                    x -= (position.x + vulnerable_area_box.w) - bg_pos_with_w;
                } else {
                    x = bg_pos.x;
                }
            }
            //if (!is_player()) std::cout << "### HITAREA (custom vulnerable-area, with BG) - NPC[" << name << "] - direction[" << (int)state.direction() << "], x[" << x << "], y[" << y << "], pos.x[" << get_int_position().x << "], real.pos.x[" << realPosition.x << "], bg.pos.x[" << GameMediator::get_instance()->get_enemy(_number)->sprites_pos_bg.x << "], graph.w[" << graphLib.character_graphics_background_list.find(name)->second.width << "]" << std::endl;
        }
        //if (!is_player()) std::cout << "### HITAREA (custom vulnerable-area) - NPC[" << name << "] - x[" << x << "], y[" << y << "], graph.w[" << graphLib.character_graphics_background_list.find(name)->second.width << "]" << std::endl;
        w = vulnerable_area_box.w;
        h = vulnerable_area_box.h;
        return st_rectangle(x, y, w, h);
    } else {
        //if (!is_player()) std::cout << "### HITAREA (no-bg, no-hitarea custom) - NPC[" << name << "]" << std::endl;
        return get_hitbox();
    }
}


int character::get_teleport_state()
{
    return teleporting_out;
}


bool character::is_in_stairs_frame() const
{
	if (state.animation_type() == ANIM_TYPE_STAIRS || state.animation_type() == ANIM_TYPE_STAIRS_MOVE || state.animation_type() == ANIM_TYPE_STAIRS_SEMI || state.animation_type() == ANIM_TYPE_STAIRS_ATTACK) {
		return true;
	}
    return false;
}


bool character::get_can_fly()
{
    return can_fly;
}



st_position character::get_int_position()
{
    return st_position((int)position.x, (int)position.y);
}


int character::get_direction() const
{
	return state.direction();
}


void character::set_direction(int direction)
{
    int must_adjust_x = frameSize.width % TILESIZE;
    if (!is_player() && direction != state.direction() && must_adjust_x != 0) {
        // fix to avoid getting stuck into a wall //
        if (direction == ANIM_DIRECTION_LEFT) {
            position.x -= TILESIZE/3;
        } else {
            position.x += TILESIZE/3;
        }
    }
    if (direction != state.direction()) {
        reset_dash_effect();
    }
    state.set_direction(direction);
}





Uint8 character::get_current_hp() const
{
	return hitPoints.current;
}



st_position character::get_real_position() const
{
	return realPosition;
}




short character::get_anim_type() const
{
	return state.animation_type();
}


short character::get_number() const
{
	return _number;
}



void character::set_progressive_appear_pos(int pos)
{
    _progressive_appear_pos = pos;
}


bool character::is_stage_boss()
{
    return _is_stage_boss;
}


bool character::is_final_game_boss()
{
    return _is_final_game_boss;
}


std::string character::replay_trace()
{
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s pos=%.3f,%.3f anim=%d/%d dir=%d hp=%d jump=%d fall=%d platform=%d inv=%d shots=%u",
             name.c_str(), (double)position.x, (double)position.y, (int)state.animation_type(), (int)state.animation_state(),
             (int)state.direction(), (int)hitPoints.current, (int)_obj_jump.is_started(), (int)_is_falling,
             (int)(_platform != NULL), (int)state.invisible, get_projectile_count());
    return std::string(buffer);
}
