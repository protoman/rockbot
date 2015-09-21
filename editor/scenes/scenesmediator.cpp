#include "scenesmediator.h"

ScenesMediator* ScenesMediator::_instance = NULL;

ScenesMediator::ScenesMediator()
{
    selected_scene = 0;
}

ScenesMediator *ScenesMediator::get_instance()
{
    if (_instance == NULL) {
        _instance = new ScenesMediator;
    }
    return _instance;
}

