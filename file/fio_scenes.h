#ifndef FIO_SCENES_H
#define FIO_SCENES_H

#include <cstdlib>
#include <iostream>
#include <vector>
#include <map>

#include "file/v4/file_scene_v4.h"

extern std::string FILEPATH; /**< TODO */

struct st_game_scene_item {
    Uint8 id;
    Uint8 scene_id;
    st_game_scene_item() {
        id = 0;
        scene_id = 0;
    }
    st_game_scene_item(Uint8 set_id, Uint8 set_scene_id) {
        id = set_id;
        scene_id = set_scene_id;
    }
};


class fio_scenes
{
public:
};

namespace format_v4 {



    class fio_scenes
    {
    public:
        fio_scenes();
        std::vector<file_scene_list> load_scenes();
        std::vector<file_scene_clear_area> load_scenes_clear_area();
        std::vector<file_scene_play_music> load_scenes_play_music();
        std::vector<file_scene_play_sfx> load_scenes_play_sfx();
        std::vector<file_scene_show_animation> load_scenes_show_animation();
        std::vector<file_scene_show_image> load_scenes_show_image();
        std::vector<file_scene_show_viewpoint> load_scenes_show_viewpoint();
        std::vector<file_scene_show_text> load_scenes_show_text();
        std::map<Uint8, Uint8> load_game_scenes();

        void save_scenes_show_image(std::vector<file_scene_show_image> data);
        void save_scenes_show_viewpoint(std::vector<file_scene_show_viewpoint> data);
        void save_scenes_show_text(std::vector<file_scene_show_text> data);
        void save_scenes(std::vector<file_scene_list> data);
        void save_scenes_show_animation(std::vector<file_scene_show_animation> data);
        void save_scenes_play_sfx(std::vector<file_scene_play_sfx> data);
        void save_scenes_play_music(std::vector<file_scene_play_music> data);
        void save_scenes_clear_area(std::vector<file_scene_clear_area> data);
        void save_game_scenes(std::map<Uint8, Uint8> data);

        void generate_files();


    private:
        template <class T> std::vector<T> load_from_disk(std::string file);
        template <class T> void save_data_to_disk(std::string file, std::vector<T> data);
    };
}

#endif // FIO_SCENES_H
