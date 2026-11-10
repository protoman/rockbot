// character: sprites, animation frames and drawing (split from character.cpp, no change of behaviour)
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




// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void character::change_char_color(Sint8 colorkey_n, st_color new_color, bool full_change=true)
{
    if (sprite_set() == NULL) {
        return;
    }
    if (full_change == false) {
        graphLib.change_surface_color(colorkey_n, new_color, &(*sprite_set()).frames[state.direction()][state.animation_type()][state.animation_state()].frameSurface);
	} else {
        for (int i=0; i<2; i++) {
			for (int j=0; j<ANIM_TYPE_COUNT; j++) {
				for (int k=0; k<ANIM_FRAMES_COUNT; k++) {
                    graphLib.change_surface_color(colorkey_n, new_color, &(*sprite_set()).frames[i][j][k].frameSurface);
				}
			}
		}
    }
}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void character::advance_frameset()
{

    if (state.direction() > CHAR_ANIM_DIRECTION_COUNT) {
        set_direction(ANIM_DIRECTION_LEFT);
		return;
	}
    if (state.animation_type() > ANIM_TYPE_COUNT) {
		return;
	}
    if ((is_player() && state.animation_state() > MAX_PLAYER_SPRITES) || (!is_player() && state.animation_state() > MAX_NPC_SPRITES)) {
		state.set_animation_state(0);
		return;
	}
    if (have_frame_graphic(state.direction(), state.animation_type(), state.animation_state()) == false) {
        _was_animation_reset = true;
        state.set_animation_state(0);
        _is_last_frame = true;
    } else {
        if (have_frame_graphic(state.direction(), state.animation_type(), state.animation_state()+1) == false) {
            _is_last_frame = true;
        } else {
            _is_last_frame = false;
        }
    }
}




// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void character::show() {
    if (is_dead() == true) {
        //if (!is_player()) std::cout << "CHAR::show[" << name << "] - LEAVE #1" << std::endl;
        return;
    }

	// update real position
	char_update_real_position();
    if (!is_player() && is_on_visible_screen() == false) {
        //if (!is_player()) std::cout << "CHAR::show[" << name << "] - LEAVE #2" << std::endl;
		return;
	}

    /*
    if (is_player() == false) {
        animation_obj.show_sprite(realPosition);
    } else {
        show_at(realPosition);
    }
    */

    show_previous_sprites();

    show_at(realPosition);
}


void character::show_previous_sprites()
{
    if (must_show_dash_effect == false && state.animation_type() != ANIM_TYPE_SLIDE) {
        reset_dash_effect();
        return;
    }

    if (dash_effect_shadow_surface_frame.is_null()) {
        graphicsLib_gSurface *surface_frame_original = get_current_frame_surface(state.direction(), state.animation_type(), state.animation_state());
        // make a copy of the frame
        graphLib.initSurface(st_size(surface_frame_original->width, surface_frame_original->height), &dash_effect_shadow_surface_frame);
        graphLib.copyArea(st_position(0, 0), surface_frame_original, &dash_effect_shadow_surface_frame);

    }
    // show previous frames
    for (int i=0; i<previous_position_list.size(); i++) {
        // only show each two frames
        if (i%2 == 0) {
            continue;
        }
        st_float_position screen_pos = get_screen_position_from_point(previous_position_list.at(i));
        graphLib.set_surface_alpha_nocolorkey(40*i/2, dash_effect_shadow_surface_frame);
        graphLib.showSurfaceAt(&dash_effect_shadow_surface_frame, st_position(screen_pos.x, screen_pos.y), false);
    }
    graphLib.set_surface_alpha_nocolorkey(255, dash_effect_shadow_surface_frame);
}


void character::reset_dash_effect()
{
    dash_effect_shadow_surface_frame.freeGraphic();
}


void character::show_at(st_position pos)
{
    // check attack frame

    if (_attack_frame_n != -1 && is_on_attack_frame() && state.animation_state() == _attack_frame_n) {
        _is_attack_frame = true;
    } else {
        _is_attack_frame = false;
    }

    // show background, if any
    if (is_player() == false && have_background_graphics() == true) {
        st_position bg_pos = st_position(pos.x-background_pos.x, pos.y-background_pos.y);
        //std::cout << "### SHOW - bg_pos.x[" << bg_pos.x << "]" << std::endl;
        if (state.direction() != ANIM_DIRECTION_LEFT) {
            graphLib.showSurfaceAt(&(graphLib.character_graphics_background_list.find(name)->second), bg_pos, false);
        } else {
            graphLib.showSurfaceAt(&(graphLib.character_graphics_background_list_left.find(name)->second), bg_pos, false);
        }
        if (state.direction() == ANIM_DIRECTION_LEFT) {
            // calcular distância com base largura total, pq é inverttido
            int original_pos_x = pos.x - background_pos.x;
            int graph_diff_x = graphLib.character_graphics_background_list.find(name)->second.width-background_pos.x - frameSize.width;
            //std::cout << "char[" << name << "], original_pos_x[" << original_pos_x << "], pos.x[" << pos.x << "], background_pos.x[" << background_pos.x << "], graph_diff_x[" << graph_diff_x << "]" << std::endl;
            pos.x = original_pos_x + graph_diff_x;
        }
    }


    // only advance if time for the current frame has finished
    advance_frameset();

    // turn is a special case, if it does not exist, we must show stand instead
    if ((state.animation_type() == ANIM_TYPE_TURN || state.animation_type() == ANIM_TYPE_VERTICAL_TURN) && have_frame_graphic(state.direction(), state.animation_type(), state.animation_state()) == false) {
        if (have_frame_graphic(state.direction(), ANIM_TYPE_WALK, state.animation_state()) == true) {
            show_sprite_graphic(state.direction(), ANIM_TYPE_WALK, state.animation_state(), pos);
        } else {
            show_sprite_graphic(state.direction(), ANIM_TYPE_STAND, state.animation_state(), pos);
        }
    // npc teleport use shows stand for now (will have a common graphic to show in the future)
    } else {
        show_sprite_graphic(state.direction(), state.animation_type(), state.animation_state(), pos);
    }
    st_rectangle hitbox = get_hitbox();
    if (gameControl.get_current_map_obj() != NULL) {
        hitbox.x -= gameControl.get_current_map_obj()->getMapScrolling().x;
    }
    if (is_player() == false) {
        if (freeze_weapon_effect != FREEZE_EFFECT_NPC || is_weak_to_freeze() == false) {
            show_sprite();
#ifdef SHOW_HITBOXES
        graphLib.draw_rectangle(hitbox, 0, 255, 0, 100);
#endif
        }
    } else {
        show_sprite();
#ifdef SHOW_HITBOXES
        graphLib.draw_rectangle(hitbox, 0, 255, 0, 100);
#endif
    }
#ifdef SHOW_VULNERABLE_AREAS
    st_rectangle vulnerable_area = get_vulnerable_area();
    if (!is_player()) {
        vulnerable_area.x -= gameControl.get_current_map_obj()->getMapScrolling().x;
        if (name == "OCTOPUS") /* std::cout << "DRAW_VUL-AREA[" << name << "]pos[" << position.x - gameControl.get_current_map_obj()->getMapScrolling().x << "][" << position.y << "], vul-area[" << vulnerable_area.x << "][" << vulnerable_area.y << "][" << vulnerable_area.w << "][" << vulnerable_area.h << "], scroll.x[" << gameControl.get_current_map_obj()->getMapScrolling().x << "]" << std::endl */;
        graphLib.draw_rectangle(vulnerable_area, 255, 0, 0, 180);
    }
#endif
}



void character::show_sprite()
{
    unsigned int now_timer = timer.getTimer();
    if (state.animation_timer() < now_timer) { // time passed the value to advance frame

		// change animation state to next frame
		int frame_inc = 1;
		if (state.animation_inverse() == true) {
            frame_inc = frame_inc * -1;
		}
        int new_frame = (state.animation_state() + frame_inc);


        if (have_frame_graphic(state.direction(), state.animation_type(), new_frame)) {
			state.set_animation_state(state.animation_state() + frame_inc);
            if (state.animation_state() < 0) {
                if (state.animation_inverse() == true) {
                    advance_to_last_frame();
                } else {
                    state.set_animation_state(0);
                }
            }
        } else {
            if (state.animation_type() == ANIM_TYPE_VERTICAL_TURN) {
				if (state.direction() == ANIM_DIRECTION_LEFT) {
                    set_direction(ANIM_DIRECTION_RIGHT);
				} else {
                    set_direction(ANIM_DIRECTION_LEFT);
				}
                set_animation_type(ANIM_TYPE_STAND);
			}
			if (state.animation_inverse() == false) {
				if (state.animation_state() > 0) {
                    state.set_animation_state(0);
				}
			} else {
				advance_to_last_frame();
            }

            _was_animation_reset = true;
            // some animation types reset to stand/other
            if (state.animation_type() == ANIM_TYPE_STAIRS_ATTACK) {
                set_animation_type(ANIM_TYPE_STAIRS);
            }
        }
		if (state.animation_type() == ANIM_TYPE_WALK_ATTACK) {
            state.set_animation_timer(timer.getTimer() + 180);
		} else {
            short direction = ANIM_DIRECTION_RIGHT;
            int delay = (*sprite_set()).frame_delay(direction, state.animation_type(), state.animation_state());
            state.set_animation_timer(timer.getTimer() + delay);
        }
    }
}


// we need to reset the time of the animation to discount pause
// because otherwise, we can't animate player/enemies during a pause like transition
void character::reset_sprite_animation_timer()
{
    if (state.animation_type() == ANIM_TYPE_WALK_ATTACK) {
        state.set_animation_timer(timer.getTimer() + 180);
    } else {
        short direction = ANIM_DIRECTION_RIGHT;

        int delay = 100;
        if (sprite_set() != NULL) {
            if (direction < CHAR_ANIM_DIRECTION_COUNT) {
                if (state.animation_type() < ANIM_TYPE_COUNT) {
                    if (state.animation_state() < ANIM_FRAMES_COUNT) {
                        delay = (*sprite_set()).frame_delay(direction, state.animation_type(), state.animation_state());
                    }
                }
            }
        }
        state.set_animation_timer(timer.getTimer() + delay);
    }
}


void character::show_sprite_graphic(short direction, short type, short frame_n, st_position frame_pos)
{

    if (state.invisible == true) {
        return;
    }

    graphicsLib_gSurface *frame_surface = get_current_frame_surface(direction, type, frame_n);

    if (frame_surface == NULL) {
        return;
    }

    /// blinking when hit
    unsigned int now_timer = timer.getTimer();
    if (now_timer < hit_duration+last_hit_time) {

        if (hit_animation_timer > now_timer) {
            //graphLib.show_white_surface_at(&it_graphic->second.frames[direction][type][frame_n].frameSurface, frame_pos);
            graphLib.show_white_surface_at(frame_surface, frame_pos);
            hit_animation_count = 0;
            return;
        } else if ((hit_animation_timer+HIT_BLINK_ANIMATION_LAPSE) < now_timer) {
            hit_animation_count++;
            if (hit_animation_count > 2) {
                hit_animation_timer = now_timer+HIT_BLINK_ANIMATION_LAPSE;
            }
            return;
        }
    }
    if (_progressive_appear_pos == 0) {
        //graphLib.showSurfaceAt(&it_graphic->second.frames[direction][type][frame_n].frameSurface, frame_pos, false);
        graphLib.showSurfaceAt(frame_surface, frame_pos, false);
    } else {
        int diff_y = frameSize.height-_progressive_appear_pos;

        //graphLib.showSurfaceRegionAt(&it_graphic->second.frames[direction][type][frame_n].frameSurface, st_rectangle(0, 0, frameSize.width, (frameSize.height-_progressive_appear_pos)), st_position(frame_pos.x, frame_pos.y-diff_y));
        graphLib.showSurfaceRegionAt(frame_surface, st_rectangle(0, 0, frameSize.width, (frameSize.height-_progressive_appear_pos)), st_position(frame_pos.x, frame_pos.y-diff_y));
        _progressive_appear_pos--;
        if (_progressive_appear_pos == 0) {
            position.y -= frameSize.height;
        }
    }
}


graphicsLib_gSurface *character::get_current_frame_surface(short direction, short type, short frame_n)
{

    if (frame_n < 0) {
        frame_n = 0;
    }

    st_char_sprite_data *sprites = sprite_set();
    if (sprites == NULL) {
        std::cout << "ERROR: #1 character::show_sprite_graphic - Could not find graphic for NPC [" << name << "]" << std::endl;
        return NULL;
    }
    // for non left-right directions, use the original facing direction for NPCs
    if (is_player() == false && direction != ANIM_DIRECTION_LEFT && direction != ANIM_DIRECTION_RIGHT) {
        direction = facing;
    }
    if (have_frame_graphic(direction, type, frame_n) == false) { // check if we can find the graphic with the given N position
        if (frame_n == 0) {
            if (type == ANIM_TYPE_TELEPORT) {
                type = ANIM_TYPE_JUMP;
            } else {
                type = ANIM_TYPE_STAND;
            }
        } else {
            frame_n = 0;
            state.set_animation_state(0);
            _was_animation_reset = true;
            return &sprites->frames[direction][type][frame_n].frameSurface;
        }
        state.set_animation_state(0);
        _was_animation_reset = true;


        if (have_frame_graphic(direction, type, frame_n) == false) { // check if we can find the graphic with the given type
            set_animation_type(ANIM_TYPE_STAND);
            type = ANIM_TYPE_STAND;
            if (have_frame_graphic(direction, type, frame_n) == false) { // check if we can find the graphic at all
                std::cout << "ERROR: #2 character::show_sprite_graphic - Could not find graphic for NPC [" << name << "] at pos[0][0][0]" << std::endl;
                return NULL;
            }
        }
    }

    return &sprites->frames[direction][type][frame_n].frameSurface;
}




// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void character::addSpriteFrame(int anim_type, int posX, graphicsLib_gSurface &spritesSurface, int delay)
{
	struct st_rectangle spriteArea;

	spriteArea.x = posX*frameSize.width;
    spriteArea.y = 0;
	spriteArea.w = frameSize.width;
    spriteArea.h = frameSize.height;

    // ANIM_TYPE_STAIRS_MOVE and ANIM_TYPE_STAIRS_SEMI have an extra frame that is the mirror of the first one

    for (int anim_direction=0; anim_direction<=1; anim_direction++) {
        for (int i=0; i<ANIM_FRAMES_COUNT; i++) { // find the last free frame
            if ((*sprite_set()).frames[anim_direction][anim_type][i].frameSurface.get_surface() == NULL) {

                st_spriteFrame *sprite = &(*sprite_set()).frames[anim_direction][anim_type][i];
                graphicsLib_gSurface gsurface = graphLib.surfaceFromRegion(spriteArea, spritesSurface);

                // RIGHT
                if (anim_direction != 0) {
                    graphLib.set_spriteframe_surface(sprite, gsurface);
                // LEFT
                } else {
                    graphicsLib_gSurface gsurface_flip;
                    graphLib.flip_image(gsurface, gsurface_flip, flip_type_horizontal);
                    graphLib.set_spriteframe_surface(sprite, gsurface_flip);
                }


                (*sprite_set()).frames[anim_direction][anim_type][i].frameSurface.init_colorkeys();
                (*sprite_set()).frames[anim_direction][anim_type][i].delay = delay;

                if (anim_type == ANIM_TYPE_STAIRS_MOVE || anim_type == ANIM_TYPE_STAIRS_SEMI) {
                    st_spriteFrame *sprite = &(*sprite_set()).frames[anim_direction][anim_type][i+1];
                    if (anim_direction != 0) {
                        graphicsLib_gSurface gsurface_flip;
                        graphLib.flip_image(gsurface, gsurface_flip, flip_type_horizontal);
                        graphLib.set_spriteframe_surface(sprite, gsurface_flip);
                    } else {
                        graphLib.set_spriteframe_surface(sprite, gsurface);
                    }
                    (*sprite_set()).frames[anim_direction][anim_type][i+1].frameSurface.init_colorkeys();
                    (*sprite_set()).frames[anim_direction][anim_type][i+1].delay = delay;
                }


                break;
            }
        }
    }
}





st_char_sprite_data *character::sprite_set()
{
    if (_sprite_set_generation != graphLib.character_graphics_generation || _sprite_set_name != name) {
        std::map<std::string, st_char_sprite_data>::iterator it = graphLib.character_graphics_list.find(name);
        _sprite_set = (it != graphLib.character_graphics_list.end()) ? &(it->second) : NULL;
        _sprite_set_generation = graphLib.character_graphics_generation;
        _sprite_set_name = name;
    }
    return _sprite_set;
}


// ********************************************************************************************** //
// adds an entry into character_graphics_list map, if needed                                      //
// ********************************************************************************************** //
void character::add_graphic()
{
    if (name == "") {
        return;
    }

    std::map<std::string, st_char_sprite_data>::iterator it;
    const std::string temp_name(name);

    it = graphLib.character_graphics_list.find(name);
    if (it == graphLib.character_graphics_list.end()) { // there is no graphic with this key yet, add it
        std::pair<std::string, st_char_sprite_data> temp_data(temp_name, st_char_sprite_data());
        graphLib.character_graphics_list.insert(temp_data);
        graphLib.character_graphics_generation++;
    }
}



bool character::have_frame_graphics()
{
    std::map<std::string, st_char_sprite_data>::iterator it;
    it = graphLib.character_graphics_list.find(name);
    if (it != graphLib.character_graphics_list.end()) { // there is no graphic with this key yet, add it
        for (int i=0; i<2; i++) {
			for (int j=0; j<ANIM_TYPE_COUNT; j++) {
				for (int k=0; k<ANIM_FRAMES_COUNT; k++) {
                    if ((*sprite_set()).frames[i][j][k].frameSurface.width > 0 && (*sprite_set()).frames[i][j][k].frameSurface.get_surface() != NULL) {
						return true;
					}
				}
			}
		}
	}
    return false;
}


void character::clean_character_graphics_list()
{
    if (is_player()) {
        return;
    }
    if (graphLib.character_graphics_list.size() <= 0) {
        return;
    }

    std::map<std::string, st_char_sprite_data>::iterator it;
    it = graphLib.character_graphics_list.find(name);
    if (it != graphLib.character_graphics_list.end()) {
        graphLib.character_graphics_list.erase(it);
        graphLib.character_graphics_generation++;
    }
}


bool character::have_background_graphics()
{
    static std::map<std::string, graphicsLib_gSurface>::iterator it;
    it = graphLib.character_graphics_background_list.find(name);
    if (it != graphLib.character_graphics_background_list.end()) { // there is no graphic with this key yet, add it
        return true;
    }
    return false;
}



int character::frames_count()
{
    return (*sprite_set()).frames_count(state.direction(), state.animation_type());
}


void character::advance_to_last_frame()
{
	int frames_n = frames_count();
	if (frames_n > 0) {
		state.set_animation_state(frames_n - 1);
	} else {
		state.set_animation_state(0);
    }
}


bool character::is_on_last_animation_frame()
{
    bool result = (_is_last_frame && state.animation_timer() < timer.getTimer());
    return result;
}


bool character::have_frame_graphic(int direction, int type, int pos)
{
    return (*sprite_set()).has_frame(direction, type, pos);
}


bool character::is_on_attack_frame()
{
    if (state.animation_type() == ANIM_TYPE_ATTACK ||
        state.animation_type() == ANIM_TYPE_STAIRS_ATTACK ||
        state.animation_type() == ANIM_TYPE_WALK_ATTACK ||
        state.animation_type() == ANIM_TYPE_JUMP_ATTACK ||
        state.animation_type() == ANIM_TYPE_THROW ||
        state.animation_type() == ANIM_TYPE_SPECIAL_ATTACK ||
        state.animation_type() == ANIM_TYPE_ATTACK_DIAGONAL_DOWN ||
        state.animation_type() == ANIM_TYPE_ATTACK_DIAGONAL_UP ||
        state.animation_type() == ANIM_TYPE_ATTACK_DOWN ||
        state.animation_type() == ANIM_TYPE_ATTACK_UP ||
        state.animation_type() == ANIM_TYPE_ATTACK_SPECIAL) {
        return true;
    }
    return false;
}


bool character::animation_has_restarted()
{
    return _was_animation_reset;
}


void character::set_animation_has_restarted(bool restarted)
{
    _was_animation_reset = restarted;
}


graphicsLib_gSurface *character::get_char_frame(int direction, int type, int frame)
{
    if (sprite_set() == NULL) {
		return NULL;
	} else {
        return &(*sprite_set()).frames[direction][type][frame].frameSurface;
	}
}


st_color character::get_color_key(short key_n) const
{
	return color_keys[key_n];
}


void character::set_animation_type(ANIM_TYPE type)
{
    // if is hit, finish jumping
    if (state.animation_type() != type && type == ANIM_TYPE_HIT) {
        _obj_jump.finish();
    }

    if (type != state.animation_type()) {
        state.set_animation_state(0);

        if (is_in_stairs_frame() && type == ANIM_TYPE_HIT) {
            if (state.direction() == ANIM_DIRECTION_RIGHT) {
                position.x += 2;
            }
        }

        state.set_animation_type(type);
        _was_animation_reset = false;


        // avoids slides starting inside wall or object
        if (type == ANIM_TYPE_SLIDE) {
            if (state.direction() == ANIM_DIRECTION_LEFT) {
                position.x += 6;
            } else {
                position.x -= 6;
            }
        }
    }
    int frame_delay = 20;
    if (sprite_set() != NULL) {
        if (state.direction() >= CHAR_ANIM_DIRECTION_COUNT) {
            set_direction(0);
        }
        if (state.animation_type() >= ANIM_TYPE_COUNT) {
            set_animation_type(ANIM_TYPE_STAND);
        }
        if (state.animation_state() >= ANIM_FRAMES_COUNT) {
            state.set_animation_state(0);
        }

        frame_delay = (*sprite_set()).frame_delay(state.direction(), state.animation_type(), state.animation_state());
    }
    state.set_animation_timer(timer.getTimer() + frame_delay);
    animation_obj.set_type(static_cast<ANIM_TYPE>(state.animation_type()));
}


void character::set_animation_frame(unsigned int frame)
{
    state.set_animation_state(frame);
}
