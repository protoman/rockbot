#include <fstream>
#include <iostream>

#include "fio_scenes.h"
#include "../aux_tools/exception_manager.h"
#include "../aux_tools/stringutils.h"

namespace format_v4 {

// The scene .dat files are raw little-endian structs written by the editor.
// Big-endian targets (Wii, PS3) must byte-swap the multi-byte fields after
// loading, otherwise e.g. delay_after = 100 becomes 0x64000000 and the intro
// waits for ~19 days.
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    namespace {
        inline void sw(int &v) { v = (int)__builtin_bswap32((unsigned int)v); }
        inline void sw(Uint16 &v) { v = __builtin_bswap16(v); }

        void convert(st_scene_area &a) { sw(a.x); sw(a.y); sw(a.w); sw(a.h); }

        void convert(file_scene_clear_area &s) {
            sw(s.x); sw(s.y); sw(s.w); sw(s.h); sw(s.r); sw(s.g); sw(s.b);
        }
        void convert(file_scene_show_image &s) {
            sw(s.ini_x); sw(s.ini_y); sw(s.dest_x); sw(s.dest_y);
            convert(s.copy_area);
            sw(s.move_delay); sw(s.loop_mode); sw(s.move_type);
        }
        void convert(file_scene_show_viewpoint &s) {
            sw(s.ini_x); sw(s.ini_y); sw(s.dest_x); sw(s.dest_y);
            sw(s.w); sw(s.h); sw(s.pos_x); sw(s.pos_y);
            sw(s.move_delay); sw(s.loop_mode);
        }
        void convert(file_scene_show_animation &s) {
            sw(s.x); sw(s.y); sw(s.frame_w); sw(s.frame_h); sw(s.frame_delay);
        }
        void convert(file_scene_show_text &s) {
            for (int i = 0; i < SCENE_TEXT_LINES_N; i++) { sw(s.line_string_id[i]); }
            sw(s.x); sw(s.y); sw(s.transition_type); sw(s.position_type);
        }
        void convert(file_scene_play_sfx &s) { sw(s.repeat_times); }
        void convert(file_scene_play_music &) {}
        void convert(file_scene_show_parallax &s) {
            sw(s.frame_delay); sw(s.loop_mode); sw(s.move_direction);
            sw(s.total_duration); sw(s.TBD2);
            for (int i = 0; i < PARALLAX_LAYERS_MAX; i++) {
                sw(s.layer_speed[i]); sw(s.adjust_y[i]); sw(s.adjust_h[i]);
                sw(s.TBD3[i]); sw(s.TBD4[i]);
            }
        }
        void convert(file_scene_list &s) {
            for (int i = 0; i < SCENE_OBJECTS_MAX; i++) {
                sw(s.objects[i].seek_n); sw(s.objects[i].delay_after);
                sw(s.objects[i].repeat_type); sw(s.objects[i].repeat_value);
            }
        }

        template <class T> std::vector<T> to_host(std::vector<T> v) {
            for (size_t i = 0; i < v.size(); i++) { convert(v[i]); }
            return v;
        }
    }
#else
    namespace {
        template <class T> std::vector<T> to_host(std::vector<T> v) { return v; }
    }
#endif

    fio_scenes::fio_scenes()
    {

    }

    std::vector<file_scene_list> fio_scenes::load_scenes()
    {
        return to_host<file_scene_list>(load_from_disk<file_scene_list>("scenes_list.dat"));
    }

    std::vector<file_scene_clear_area> fio_scenes::load_scenes_clear_area()
    {
        return to_host<file_scene_clear_area>(load_from_disk<file_scene_clear_area>("scenes_clear_area.dat"));
    }

    std::vector<file_scene_play_music> fio_scenes::load_scenes_play_music()
    {
        return to_host<file_scene_play_music>(load_from_disk<file_scene_play_music>("scenes_play_music.dat"));
    }

    std::vector<file_scene_play_sfx> fio_scenes::load_scenes_play_sfx()
    {
        return to_host<file_scene_play_sfx>(load_from_disk<file_scene_play_sfx>("scenes_play_sfx.dat"));
    }

    std::vector<file_scene_show_animation> fio_scenes::load_scenes_show_animation()
    {
        return to_host<file_scene_show_animation>(load_from_disk<file_scene_show_animation>("scenes_show_animation.dat"));
    }

    std::vector<file_scene_show_image> fio_scenes::load_scenes_show_image()
    {
        return to_host<file_scene_show_image>(load_from_disk<file_scene_show_image>("scenes_show_image.dat"));
    }

    std::vector<file_scene_show_parallax> fio_scenes::load_scenes_parallax()
    {
        return to_host<file_scene_show_parallax>(load_from_disk<file_scene_show_parallax>("scenes_parallax.dat"));
    }

    std::vector<file_scene_show_viewpoint> fio_scenes::load_scenes_show_viewpoint()
    {
        return to_host<file_scene_show_viewpoint>(load_from_disk<file_scene_show_viewpoint>("scenes_show_viewpoint.dat"));
    }

    std::vector<file_scene_show_text> fio_scenes::load_scenes_show_text()
    {
        return to_host<file_scene_show_text>(load_from_disk<file_scene_show_text>("scenes_show_text.dat"));
    }

    std::map<Uint8, Uint8> fio_scenes::load_game_scenes()
    {
        std::map<Uint8, Uint8> res;
        std::vector<st_game_scene_item> list = load_from_disk<st_game_scene_item>("game_scenes.dat");
        for (int i=0; i<list.size(); i++) {
            res.insert(std::pair<Uint8, Uint8>(list.at(i).id, list.at(i).scene_id));
        }
        return res;
    }

    void fio_scenes::save_scenes_show_image(std::vector<file_scene_show_image> data)
    {
        save_data_to_disk<file_scene_show_image>("scenes_show_image.dat", data);
    }

    void fio_scenes::save_scenes_parallax(std::vector<file_scene_show_parallax> data)
    {
        save_data_to_disk<file_scene_show_parallax>("scenes_parallax.dat", data);
    }

    void fio_scenes::save_scenes_show_viewpoint(std::vector<file_scene_show_viewpoint> data)
    {
        save_data_to_disk<file_scene_show_viewpoint>("scenes_show_viewpoint.dat", data);
    }

    void fio_scenes::save_scenes_show_text(std::vector<file_scene_show_text> data)
    {
        save_data_to_disk<file_scene_show_text>("scenes_show_text.dat", data);
    }

    void fio_scenes::save_scenes(std::vector<file_scene_list> data)
    {
        save_data_to_disk<file_scene_list>("scenes_list.dat", data);
    }

    void fio_scenes::save_scenes_show_animation(std::vector<file_scene_show_animation> data)
    {
        save_data_to_disk<file_scene_show_animation>("scenes_show_animation.dat", data);
    }

    void fio_scenes::save_scenes_play_sfx(std::vector<file_scene_play_sfx> data)
    {
        save_data_to_disk<file_scene_play_sfx>("scenes_play_sfx.dat", data);
    }

    void fio_scenes::save_scenes_play_music(std::vector<file_scene_play_music> data)
    {
        save_data_to_disk<file_scene_play_music>("scenes_play_music.dat", data);
    }

    void fio_scenes::save_scenes_clear_area(std::vector<file_scene_clear_area> data)
    {
        save_data_to_disk<file_scene_clear_area>("scenes_clear_area.dat", data);
    }

    void fio_scenes::save_game_scenes(std::map<Uint8, Uint8> data)
    {
        std::vector<st_game_scene_item> data_vector;
        for (std::map<Uint8, Uint8>::iterator itr = data.begin(), itr_end = data.end(); itr != itr_end; ++itr) {
            data_vector.push_back(st_game_scene_item(itr->first, itr->second));
        }
        save_data_to_disk<st_game_scene_item>("game_scenes.dat", data_vector);
    }

    void fio_scenes::generate_files()
    {
        save_scenes_show_image(std::vector<file_scene_show_image>());
        save_scenes_parallax(std::vector<file_scene_show_parallax>());
        save_scenes_show_viewpoint(std::vector<file_scene_show_viewpoint>());
        save_scenes_show_text(std::vector<file_scene_show_text>());
        save_scenes_show_animation(std::vector<file_scene_show_animation>());
        save_scenes_play_sfx(std::vector<file_scene_play_sfx>());
        save_scenes_play_music(std::vector<file_scene_play_music>());
        save_scenes_clear_area(std::vector<file_scene_clear_area>());

        save_scenes(std::vector<file_scene_list>());
        save_game_scenes(std::map<Uint8, Uint8>());

    }

    template <class T> std::vector<T> fio_scenes::load_from_disk(std::string file)
    {
        std::string filename = std::string(FILEPATH) + "scenes/" + file;
        std::vector<T> res;
        if (FILEPATH.length() == 0) {
            return res;
        }
        FILE *fp = fopen(StringUtils::clean_filename(filename).c_str(), "rb");
        if (!fp) {
            std::cout << "ERROR: file_io::load_from_disk - file '" << filename << "' not found." << std::endl;
            return res;
        }

        while (!feof(fp) && !ferror(fp)) {
            T out;
            size_t res_read = fread(&out, sizeof(T), 1, fp);
            if (res_read != 1) {
                break;
            } else {
                res.push_back(out);
            }
        }
        fclose(fp);
        return res;
    }

    template <class T> void fio_scenes::save_data_to_disk(std::string file, std::vector<T> data)
    {
        std::string filename = std::string(FILEPATH) + "scenes/" + file;
        FILE *fp = fopen(StringUtils::clean_filename(filename).c_str(), "wb");
        if (!fp) {
            std::cout << "ERROR: file_io::save_data_to_disk - file '" << filename << "' not found." << std::endl;
            return;
        }

        for (size_t i=0; i<data.size(); i++) {
            int block_size = sizeof(T);
            T data_in = data.at(i);
            fwrite(&data_in, block_size, 1, fp);
        }
        fclose(fp);
    }

}
