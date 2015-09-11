#include "scenesmediator.h"

ScenesMediator* ScenesMediator::_instance = NULL;

ScenesMediator::ScenesMediator()
{

}

ScenesMediator *ScenesMediator::get_instance()
{
    if (_instance == NULL) {
        _instance = new ScenesMediator;
    }
    return _instance;
}

