#ifndef CLOUD_SAVE_H
#define CLOUD_SAVE_H

#ifdef ANDROID

#include <SDL_android.h>


void cloud_load_game(std::string filename) {
    //SDLCALL SDL_ANDROID_CloudLoad(const char *filename, const char *saveId, const char *dialogTitle);
    std::string save_id = "Rockbot2.1";
    SDL_ANDROID_CloudLoad(filename.c_str(), save_id.c_str(), NULL);
}

void cloud_save_game(std::string filename) {
    //extern DECLSPEC int SDLCALL SDL_ANDROID_CloudSave(const char *filename, const char *saveId,
    // const char *dialogTitle, const char *description, const char *screenshotFile, uint64_t playedTimeMs);
    std::string save_id = "Rockbot2.1";
    SDL_ANDROID_CloudSave(filename.c_str(),save_id.c_str(), NULL, NULL, NULL, SDL_GetTicks());
}

#endif // ANDROID

#endif // CLOUD_SAVE_H

