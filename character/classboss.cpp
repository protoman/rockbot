#include "classboss.h"

#include "classmap.h"

#include "timerlib.h"
extern timerLib timer;

#include "scenes/dialogs.h"

#include "defines.h"
#include <cmath>
#include <cstdlib>
#include "game.h"
extern game gameControl;

#include "soundlib.h"
extern soundLib soundManager;


extern CURRENT_FILE_FORMAT::file_game game_data;
extern FREEZE_EFFECT_TYPES freeze_weapon_effect;


classboss::classboss(std::string set_name) : _initialized(0), _screen_blinked(false)
{
	name = set_name;
    this->set_is_boss(true);
	_ai_state.initial_position.y = -200;
	position.y = _ai_state.initial_position.y;
    hitPoints.total = PLAYER_INITIAL_HP;

    hitPoints.current = hitPoints.total;
    hit_duration = BOSS_HIT_DURATION;
    std::cout << "classboss::classboss #1 [" << name << "] - is_boss: " << _is_boss << std::endl;
    _is_boss = true;
}

classboss::classboss(int stage_id, int map_id, int main_id, int id) : classnpc(stage_id, map_id, main_id, id)
{
	_initialized = 0;
	_screen_blinked = false;
	this->set_is_boss(true);
    _ai_state.initial_position.y = -(frameSize.height+1);
	position.y = _ai_state.initial_position.y;
	hitPoints.total = PLAYER_INITIAL_HP;
    hitPoints.current = hitPoints.total;
    hit_duration = BOSS_HIT_DURATION;
    //std::cout << "classboss::classboss #2 - stage_id: " << stage_id << ", map_id: " << map_id << ", main_id: " << main_id << ", id: " << id << std::endl;
}


void classboss::boss_move() {
    //std::cout << "classboss::boss_move - _initialized: " << _initialized << std::endl;

    move_projectiles();
    //std::cout << "classboss::boss_move - A" << std::endl;

    if (_initialized == 0 && _is_boss == true) { /// @TODO: move this logic to map (player should not move while boss is presenting)
        //std::cout << "classboss::boss_move - B" << std::endl;
		_initialized++;
        set_animation_type(ANIM_TYPE_TELEPORT);
        gameControl.map_present_boss(_is_boss);
        //std::cout << "classboss::boss_move - unitialized, set teleport" << std::endl;
		return;
    } else if (_initialized == 1 && _is_boss == true) {
        //std::cout << "classboss::boss_move - C" << std::endl;
        //std::cout << "classboss::boss_move - #5 - _initialized: " << _initialized << std::endl;
		if (position.x > RES_H/3 && gravity(true) == false) {
			_initialized++;
		}
		return;
	}
    //std::cout << "classboss::boss_move - D" << std::endl;


	if (first_run == 0) {
		first_run = 1;
	}

	if (_ai_timer > timer.getTimer()) {
        return;
	}

    execute_ai();
    gravity(false);
}

// ********************************************************************************************** //
//                                                                                                //
// ********************************************************************************************** //
void classboss::execute()
{
	if (hitPoints.current <= 0 || position.x < map->getMapScrolling().x-TILESIZE*2 || position.x > map->getMapScrolling().x+RES_W+TILESIZE*2) {
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
    //std::cout << "classboss::execute" << std::endl;
	boss_move();
	charMove();
}



void classboss::death()
{
    map->clear_animations();
    _obj_jump.interrupt();
    _obj_jump.finish();
    dead = true;
    graphLib.set_screen_adjust(st_position(0, 0));
}

bool classboss::is_boss()
{
    return true;
}
