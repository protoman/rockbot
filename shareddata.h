#ifndef SHAREDDATA_H
#define SHAREDDATA_H

#include "defines.h"
#include "file/file_io.h"
#include "objects/object.h"

class object; // forward declaration

class SharedData
{
public:
    static SharedData* get_instance();
    static int get_max_fps();
    static double get_movement_multiplier();

private:
    SharedData();
    SharedData(SharedData const&){};             // copy constructor is private
    SharedData& operator=(SharedData const&){ return *this; };  // assignment operator is private

private:
    static SharedData* _instance;
    static int max_fps;
    static double fps_movement_multiplier;




public:
    struct CURRENT_FILE_FORMAT::st_game_config game_config;
    std::vector<object*> active_object_list; // contains all on-screen and active objects
    double scaleX;
    double scaleY;
    st_size scale_window_size;
    bool changed_window_size;
    short current_language = LANGUAGE_AUTODETECT;
    bool is_showing_boss_intro = false;
    bool leave_stage_request = false;
    int version_number = 1;
    std::string version_number_str = "1";
    bool is_fire_tv = false;
};

#endif // SHAREDDATA_H
