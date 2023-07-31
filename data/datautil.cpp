#include "datautil.h"
#include "defines.h"
#include "file/format/st_common.h"
#include "file/v4/file_save_v4.h"

extern CURRENT_FILE_FORMAT::st_save game_save;

dataUtil* dataUtil::_instance = NULL;

dataUtil::dataUtil()
{

}

dataUtil *dataUtil::get_instance()
{
    if (!_instance) {
        _instance = new dataUtil();
    }
    return _instance;
}

bool dataUtil::has_weapon(int weapon_n) const
{
    if (weapon_n <= WEAPON_SEAHORSEBOT) {
        if (game_save.stages[weapon_n] == 1) {
            return true;
        }
    } else {
        if (weapon_n == WEAPON_ITEM_COIL && game_save.stages[WEAPON_APEBOT] == 1) {
            return true;
        } else if (weapon_n == WEAPON_ITEM_JET && game_save.stages[WEAPON_TECHNOBOT] == 1) {
            return true;
        } else if (weapon_n == WEAPON_ITEM_ETANK && game_save.items.energy_tanks > 0) {
            return true;
        } else if (weapon_n == WEAPON_ITEM_WTANK && game_save.items.weapon_tanks > 0) {
            return true;
        } else if (weapon_n == WEAPON_ITEM_STANK && game_save.items.special_tanks > 0) {
            return true;
        }
    }
    return false;
}

