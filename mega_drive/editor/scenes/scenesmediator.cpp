#include "scenesmediator.h"

ScenesMediator* ScenesMediator::_instance = NULL;

ScenesMediator::ScenesMediator()
{
    selected_scene = 0;
    load_game_scenes();
}

ScenesMediator *ScenesMediator::get_instance()
{
    if (_instance == NULL) {
        _instance = new ScenesMediator;
    }
    return _instance;
}

void ScenesMediator::save_game_scenes()
{
    fio_scn.save_game_scenes(game_scenes_map);
}

void ScenesMediator::load_game_scenes()
{
    game_scenes_map = fio_scn.load_game_scenes();
}

