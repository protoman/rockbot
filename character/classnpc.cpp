#include <cstdlib>
#include <sstream>
#include "classnpc.h"
#include "graphicslib.h"
#include "stage.h"
#include "classplayer.h"
#include "classmap.h"
#include <math.h>
#include <string.h>

#include "game_mediator.h"

#ifdef ANDROID
#include <android/log.h>
#endif

#include "inputlib.h"
extern inputLib input;

#include "game.h"
extern game gameControl;


extern std::string FILEPATH;

extern CURRENT_FILE_FORMAT::file_game game_data;
extern CURRENT_FILE_FORMAT::file_stage stage_data;
extern CURRENT_FILE_FORMAT::file_map map_data[FS_STAGE_MAX_MAPS];

extern FREEZE_EFFECT_TYPES freeze_weapon_effect;
extern int freeze_weapon_id;

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
classnpc::classnpc() : graphic_filename(), first_run(true), _is_player_friend(false)
{
	add_graphic();
	hit_duration = 500;
    if (is_subboss()) {
        hit_duration = BOSS_HIT_DURATION;
    }
	clean_projectiles();
    facing = 0;
    _is_spawn = false;
    _initialized = false;
    _screen_blinked = false;

    // can't have ghosts that don't fly
    if (is_ghost == true && can_fly == false) {
        is_ghost = false;
    }
}




classnpc::classnpc(int stage_id, int map_id, int main_id, int id) : _is_player_friend(false) // map-loaded npc
{
    build_basic_npc(stage_id, map_id, main_id);
    facing = map_data[map_id].map_npcs[id].direction;
    state.direction = facing;

    fflush(stdout);

    start_point.x = map_data[map_id].map_npcs[id].start_point.x * TILESIZE;
    start_point.y = map_data[map_id].map_npcs[id].start_point.y * TILESIZE;
    position.x = start_point.x;
    position.y = start_point.y;
    _is_spawn = false;
    _initialized = false;
    _screen_blinked = false;

    // can't have ghosts that don't fly
    if (is_ghost == true && can_fly == false) {
        is_ghost = false;
    }
}

classnpc::classnpc(int stage_id, int map_id, int main_id, st_position npc_pos, short int direction, bool player_friend) // spawned npc
{
    build_basic_npc(stage_id, map_id, main_id);
    _is_player_friend = player_friend;
    facing = direction;
    state.direction = direction;
    start_point.x = npc_pos.x;
    start_point.y = npc_pos.y;
    position.x = npc_pos.x;
    position.y = npc_pos.y;
    _is_spawn = true;
    _initialized = false;
    _screen_blinked = false;

    // can't have ghosts that don't fly
    if (is_ghost == true && can_fly == false) {
        is_ghost = false;
    }
}




// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
classnpc::classnpc(std::string set_name) : graphic_filename(), first_run(true), _is_player_friend(false)
{
	name = set_name;
	hit_duration = HIT_BLINK_ANIMATION_LAPSE;
	add_graphic();
    facing = 0;
    _is_spawn = false;
    _initialized = false;
    _screen_blinked = false;

    // can't have ghosts that don't fly
    if (is_ghost == true && can_fly == false) {
        is_ghost = false;
    }
}


// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
classnpc::~classnpc()
{
}

void classnpc::build_basic_npc(int stage_id, int map_id, int main_id)
{
    UNUSED(stage_id);
    UNUSED(map_id);
	_number = main_id;
	// TODO - usar operador igual e também para cópia de toda a classe para ela mesma
	graphicsLib_gSurface npc_sprite_surface;

#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "CLASSNPC::build_basic_npc, id[%d]", _number);
#endif

    CURRENT_FILE_FORMAT::file_npc* copyref = GameMediator::get_instance()->get_enemy(main_id);

    name = std::string(copyref->name);

#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "CLASSNPC::build_basic_npc, name#1[%s]", copyref->name);
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "CLASSNPC::build_basic_npc, name#2[%s]", name.c_str());
#endif


	add_graphic();



	// set sprites to be persistent. they will be freed only by game.cpp
    for (int i=0; i<CHAR_ANIM_DIRECTION_COUNT; i++) {
		for (int j=0; j<ANIM_TYPE_COUNT; j++) {
			for (int k=0; k<ANIM_FRAMES_COUNT; k++) {
                (graphLib.character_graphics_list.find(name)->second).frames[i][j][k].frameSurface.persistent = true;
			}
		}
	}



    hitPoints.total = GameMediator::get_instance()->get_enemy(main_id)->hp.total;
	hitPoints.current = hitPoints.total;

    if (state.direction > CHAR_ANIM_DIRECTION_COUNT) {
		state.direction = ANIM_DIRECTION_RIGHT;
	}

    move_speed = GameMediator::get_instance()->get_enemy(main_id)->speed;
    walk_range = GameMediator::get_instance()->get_enemy(main_id)->walk_range;
	if (walk_range < 0 || walk_range > 1000) { // fix data errors by setting value to default
		walk_range = TILESIZE*6;
	}



    graphic_filename = GameMediator::get_instance()->get_enemy(main_id)->graphic_filename;


    frameSize.width = GameMediator::get_instance()->get_enemy(main_id)->frame_size.width;
    frameSize.height = GameMediator::get_instance()->get_enemy(main_id)->frame_size.height;
    is_ghost = (GameMediator::get_instance()->get_enemy(main_id)->is_ghost != 0);
    shield_type = GameMediator::get_instance()->get_enemy(main_id)->shield_type;
	_is_boss = false;

    _attack_frame_n = GameMediator::get_instance()->get_enemy(main_id)->attack_frame;


    // TODO - this logic can be passed to the editor
    if (GameMediator::get_instance()->get_enemy(main_id)->fly_flag != 0) {
        //std::cout << "******** classnpc::set_file_data - npc: " << name << ", canfly: " << can_fly << std::endl;
        can_fly = true;
    }
	realPosition.x = 0;
	realPosition.y = 0;

	max_projectiles = 1;			// hardcoded, fix this in editor
	//move_speed = 6;					// hardcoded, fix this in editor
	attack_state = 0;
	last_execute_time = 0;


	// only add graphics if there is no graphic for this NPC yet
	if (have_frame_graphics() == false) {
		// load the graphic from file and set it into frameset


        std::string temp_filename = FILEPATH + "images/sprites/enemies/" + graphic_filename;
        //printf(">> temp_filename: '%s'\n", temp_filename.c_str());

        graphLib.surfaceFromFile(temp_filename, &npc_sprite_surface);
        if (npc_sprite_surface.get_surface() == NULL) {
			std::cout << "initFrames - Error loading player surface from file\n";
			return;
		}

        for (int i=0; i<ANIM_TYPE_COUNT; i++) {
            for (int j=0; j<ANIM_FRAMES_COUNT; j++) {
                if (GameMediator::get_instance()->get_enemy(main_id)->sprites[i][j].used == true) {
                    addSpriteFrame(i, GameMediator::get_instance()->get_enemy(main_id)->sprites[i][j].sprite_graphic_pos_x, npc_sprite_surface, GameMediator::get_instance()->get_enemy(main_id)->sprites[i][j].duration);
                }
            }
        }
    }


    if (have_background_graphics() == false) {
        graphicsLib_gSurface bg_surface;
        std::string bg_filename(GameMediator::get_instance()->get_enemy(main_id)->bg_graphic_filename);
        //std::cout << ">>>>>>>>> NPC[" << name << "].bg_filename: '" << bg_filename << "', length: " << bg_filename.length() << ", size: " << bg_filename.size() << std::endl;
        if (bg_filename.size() > 0) {
            std::string full_bggraphic_filename = FILEPATH + "images/sprites/backgrounds/" + bg_filename;
            std::cout << ">>>>>>>>> NPC[" << name << "].bg_filename: " << bg_filename << std::endl;
            graphLib.surfaceFromFile(full_bggraphic_filename, &bg_surface);
            if (bg_surface.get_surface() == NULL) {
                std::cout << "initFrames - Error loading NPC background surface from file '" << full_bggraphic_filename << std::endl;
                return;
            }
            graphLib.character_graphics_background_list.insert(std::pair<std::string, graphicsLib_gSurface>(name, bg_surface));
            _has_background = true;
            _frame_pos_adjust.x = GameMediator::get_instance()->get_enemy(main_id)->sprites_pos_bg.x;
            _frame_pos_adjust.y = GameMediator::get_instance()->get_enemy(main_id)->sprites_pos_bg.y;
        }
    }

    // can't have ghosts that don't fly
    if (is_ghost == true && can_fly == false) {
        is_ghost = false;
    }


    //std::cout << "end" << std::endl;
}



bool classnpc::is_able_to_fly()
{
    return can_fly;
}

bool classnpc::is_spawn()
{
    return _is_spawn;
}

bool classnpc::is_subboss()
{
    return GameMediator::get_instance()->get_enemy(_number)->is_sub_boss;
}

void classnpc::reset_position()
{
    position.x = start_point.x;
    position.y = start_point.y;
    // if the NPC uses fly/fall, it means, we need to respawn it inside the hole
    if (uses_fly_fall()) {
        position.y = RES_H + TILESIZE;
    }
}

st_position classnpc::get_start_position()
{
    return st_position(start_point.x, start_point.y);
}

st_rectangle classnpc::get_hitbox()
{
    int temp_x = position.x + GameMediator::get_instance()->get_enemy(_number)->sprites[state.animation_type][state.animation_state].collision_rect.x;
    int temp_y = position.y + GameMediator::get_instance()->get_enemy(_number)->sprites[state.animation_type][state.animation_state].collision_rect.y;
    int temp_w = GameMediator::get_instance()->get_enemy(_number)->sprites[state.animation_type][state.animation_state].collision_rect.w;
    int temp_h = GameMediator::get_instance()->get_enemy(_number)->sprites[state.animation_type][state.animation_state].collision_rect.h;

    // @TODO - invert X depending on direction

    //std::cout << "#NPC::get_hitbox[" << name << "] - x: " << temp_x << ", y: " << temp_y << ", w: " << temp_w << ", h: " << temp_h << std::endl;

    if (state.direction == ANIM_DIRECTION_LEFT) {
        temp_x = (frameSize.width - GameMediator::get_instance()->get_enemy(_number)->sprites[state.animation_type][state.animation_state].collision_rect.x) - GameMediator::get_instance()->get_enemy(_number)->sprites[state.animation_type][state.animation_state].collision_rect.w + position.x;
    }

    return st_rectangle(temp_x, temp_y, temp_w, temp_h);
}

void classnpc::show()
{
#ifdef SHOW_HITBOXES
    st_rectangle hitbox = get_hitbox();
    hitbox.x -= gameControl.get_current_map_obj()->getMapScrolling().x;
    graphLib.draw_rectangle(hitbox, 0, 0, 255, 100);
#endif
    character::show();
}




// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void classnpc::initFrames()
{
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void classnpc::execute()
{


    if (freeze_weapon_effect == FREEZE_EFFECT_NPC && is_weak_to_freeze() == true) {
        clean_projectiles();
        return;
    }

    if (is_dead() == true) {
        move_projectiles();
    } else {
        if (is_boss() || is_stage_boss()) {
            boss_move();
        } else {
            move();
        }
        if (gameControl.must_break_npc_loop == false) {
            charMove();
        }
    }
}

void classnpc::boss_move()
{
    //std::cout << "NPC::boss_move::BEGIN" << std::endl;
    if (hitPoints.current <= 0 || position.x < gameControl.get_current_map_obj()->getMapScrolling().x-TILESIZE*2 || position.x > gameControl.get_current_map_obj()->getMapScrolling().x+RES_W+TILESIZE*2) {
        //std::cout << "classboss::execute - LEAVE #1" << std::endl;
        return;
    }
    if (last_execute_time > timer.getTimer()) {
        //std::cout << "classboss::execute - LEAVE #2" << std::endl;
        return;
    }
    if (freeze_weapon_effect == FREEZE_EFFECT_NPC && is_weak_to_freeze() == true) {
        //std::cout << "classboss::execute - LEAVE #3" << std::endl;
        clean_projectiles();
        return;
    }

    //std::cout << "classboss::boss_move[" << name << "] - _initialized: " << _initialized << std::endl;

    move_projectiles();

    if (_initialized == 0 && _is_boss == true) { /// @TODO: move this logic to map (player should not move while boss is presenting)
        _initialized++;
        set_animation_type(ANIM_TYPE_TELEPORT);
        gameControl.map_present_boss(_is_boss);
        return;
    } else if (_initialized == 1 && _is_boss == true) {
#ifdef ANDROID
        __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "NPC::boss_move::GRAVITY #1, position.x[%f]", position.x);
#endif
        if (position.x > RES_H/3 && gravity(true) == false) {
            _initialized++;
        }
        return;
    }

    if (first_run == 0) {
        first_run = 1;
    }

    if (_ai_timer > timer.getTimer()) {
        return;
    }

#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "NPC::boss_move::EXECUTE_AI");
#endif
    execute_ai();
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_INFO, "###ROCKBOT2###", "NPC::boss_move::GRAVITY #2, must_break_loop[%d]", gameControl.must_break_npc_loop?1:0);
#endif
    if (gameControl.must_break_npc_loop == false) {
        gravity(false);
    }
}



void classnpc::copy(classnpc *from)
{
	int i;

	facing = from->facing;
	start_point = from->start_point;

	walk_range = from->walk_range;
	graphic_filename = from->graphic_filename;
    is_ghost = from->is_ghost;
	shield_type = from->shield_type;

	_is_boss = from->_is_boss;
	hitPoints = from->hitPoints;
	name = from->name;
	position = from->position;
	realPosition = from->realPosition;
	last_execute_time = 0;
	frameSize = from->frameSize;
	moveCommands = from->moveCommands;
	max_projectiles = from->max_projectiles;
	move_speed = from->move_speed;
    can_fly = from->can_fly;
    attack_state = 0;
	state = from->state;

}



void classnpc::move_projectiles()
{
	//int i = 0;
	// animate projectiles
    //if (name == "Dynamite Bot") std::cout << "******* NPC::move_projectiles - projectile_list.size: " << projectile_list.size() << std::endl;
    std::vector<projectile>::iterator it;
    st_rectangle player_hitbox = gameControl.get_current_map_obj()->get_player_hitbox();

	for (it=projectile_list.begin(); it<projectile_list.end(); it++) {
        (*it).draw();
        st_size moved = (*it).move();
        // check collision agains players

        if ((*it).is_reflected == true) {
			continue;
		}

        if (_is_player_friend == false) { // NPC attacking players

            if ((*it).is_finished == true) {
                projectile_list.erase(it);
                break;
            }
            if ((*it).get_move_type() == TRAJECTORY_QUAKE) {
                ground_damage_players();
                continue;
            } else if ((*it).get_move_type() == TRAJECTORY_PUSH_BACK) {
                //@TODO
                push_back_players(!state.direction);
                continue;
            }

            if ((*it).check_collision(player_hitbox, st_position(moved.width, moved.height)) == true) {
                if (gameControl.get_current_map_obj()->_player_ref->is_shielded((*it).get_direction()) == true && (*it).get_trajectory() != TRAJECTORY_BOMB && (*it).get_trajectory() != TRAJECTORY_LIGHTING) {
                    (*it).reflect();
                } else if (gameControl.get_current_map_obj()->_player_ref->is_using_circle_weapon() == true) {
                    std::cout << "NPC projectile hit player centered-weapon" << std::endl;
                    (*it).consume_projectile();
                    gameControl.get_current_map_obj()->_player_ref->consume_projectile();
                } else {
                    int damage_pts = (*it).get_damage();
                    if (damage_pts < 2) {
                        damage_pts = 2;
                    }
                    gameControl.get_current_map_obj()->_player_ref->damage(damage_pts, false);
                    (*it).consume_projectile();
                }
            }
        } else { // NPC attacking other NPCs

            for (int i=0; i<gameControl.get_current_map_obj()->_npc_list.size(); i++) {
                st_rectangle other_npc_hitbox = gameControl.get_current_map_obj()->_npc_list.at(i).get_hitbox();
				//classnpc* enemy = (*enemy_it);
                if ((*it).check_collision(other_npc_hitbox, st_position(moved.width, moved.height)) == true) {
					//std::cout << "is_shielded::CALL 2" << std::endl;
                    if (gameControl.get_current_map_obj()->_npc_list.at(i).is_intangible() == true) {
                        continue;
                    } else if (gameControl.get_current_map_obj()->_npc_list.at(i).is_shielded((*it).get_direction()) == true && (*it).get_trajectory() != TRAJECTORY_BOMB && (*it).get_trajectory() != TRAJECTORY_LIGHTING) {
                        (*it).reflect();
					} else {
                        gameControl.get_current_map_obj()->_npc_list.at(i).damage((*it).get_damage(), false);
                        if ((*it).get_move_type() != TRAJECTORY_CHAIN) { /// @TODO non-destructable types
                            (*it).consume_projectile();
						}
					}
				}
                if ((*it).is_finished == true) {
					projectile_list.erase(it);
					break;
				}
			}
		}
	}
}


// executes the NPC sub-IA behavior
void classnpc::move() {
    if (state.direction > CHAR_ANIM_DIRECTION_COUNT-1) {
        state.direction = ANIM_DIRECTION_LEFT;
    }


	/// @TODO: this check must be placed in game.cpp and npc must have a set_frozen() method, for individual effect

	move_projectiles();


	if (last_execute_time > timer.getTimer()) {
		return;
	}

	if (state.frozen == true) {
		return;
	}


    if (hitPoints.current > 0) {
        execute_ai();
	}

    //std::cout << "classnpc::move - position.y: " << position.y << std::endl;

    if (can_fly == false && position.y >= RES_H+1) {
        // death because felt in a hole
        damage(999, true);
        position.x = start_point.x;
        position.y = start_point.y;
        //std::cout << "classnpc::move - FELT IN HOLE - DEATH - pos.x: " << position.x << ", pos.y: " << position.y << std::endl;
        return;
    }

}



// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //



short classnpc::get_dead_state()
{
    if (hitPoints.current > 0) {
        _dead_state = 0;
    } else {
        if (_dead_state == 0) {
            death();
            _dead_state = 1;
        } else if (_dead_state == 1) {
            _dead_state = 2;
        }
    }
    return _dead_state;
}


void classnpc::death()
{
    _obj_jump.interrupt();
    _obj_jump.finish();
    dead = true;
    _auto_respawn_timer = timer.getTimer() + GameMediator::get_instance()->get_enemy(_number)->respawn_delay;
    //std::cout << "classnpc::death" << std::endl;
    if (_ai_state.main_status == IA_STATE_QUAKE_ATTACK) {
        graphLib.set_screen_adjust(st_position(0, 0));
    }
    if (is_stage_boss()) {
        gameControl.get_current_map_obj()->clear_animations();
    }
}

bool classnpc::is_boss()
{
	return _is_boss;
}




void classnpc::set_is_boss(bool set_boss)
{
    _is_boss = set_boss;
    if (_is_boss == true) {
        _screen_blinked = false;
        _ai_state.initial_position.y = -(frameSize.height+1);
        position.y = _ai_state.initial_position.y;
        hitPoints.total = BOSS_INITIAL_HP;
        hitPoints.current = hitPoints.total;
        hit_duration = BOSS_HIT_DURATION;
    }
}

bool classnpc::is_player_friend()
{
	return _is_player_friend;
}

void classnpc::set_stage_boss(bool boss_flag)
{
    _is_boss = boss_flag;
    _is_stage_boss = boss_flag;
    if (boss_flag == true) {
        _screen_blinked = false;
        _ai_state.initial_position.y = -(frameSize.height+1);
        position.y = _ai_state.initial_position.y;
        hitPoints.total = BOSS_INITIAL_HP;
        hitPoints.current = hitPoints.total;
        hit_duration = BOSS_HIT_DURATION;
    }
}

void classnpc::revive()
{
    //std::cout << "**** classnpc::revive[" << name << " ****" << std::endl;
	//position.x = start_point.x;
    //position.y = start_point.y;
    if (GameMediator::get_instance()->ai_list.at(_number).states[_ai_chain_n].extra_parameter == AI_ACTION_FLY_OPTION_DRILL_DOWN) {
        position.y = -TILESIZE;
    }
	hitPoints.current = hitPoints.total;
}





void classnpc::invert_direction()
{
	if (state.direction == ANIM_DIRECTION_LEFT) {
		state.direction = ANIM_DIRECTION_RIGHT;
	} else {
		state.direction = ANIM_DIRECTION_LEFT;
	}
    _ai_state.main_status = IA_STAND;
}




