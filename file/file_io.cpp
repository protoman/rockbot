#include <string.h>
#include <stdio.h>
#include <cstdlib>
#include "file_io.h"
#include "convert.h"
#include "../file/convert.h"
#include "aux_tools/stringutils.h"

#ifdef DREAMCAST
#include <kos.h>
#endif

extern std::string FILEPATH;
extern std::string SAVEPATH;
extern std::string GAMEPATH;
extern std::string GAMENAME;


// ************************************************************************************************************* //

namespace format_v4 {

    file_io::file_io()
    {
        sufix = "_v301";
    }



    void file_io::write_game(format_v4::file_game& data_in) const {
        std::ofstream fp;
        std::string filename = "";


// -------------------------------------- GAME -------------------------------------- //
        filename = std::string(FILEPATH) + "game_properties" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.version), sizeof(float));
        fp.write(reinterpret_cast<char *>(&data_in.name), sizeof(char) * FS_CHAR_NAME_SIZE);
        fp.write(reinterpret_cast<char *>(&data_in.semi_charged_projectile_id), sizeof(Sint8));
        fp.write(reinterpret_cast<char *>(&data_in.player_items), sizeof(Sint8) * FS_PLATER_ITEMS_N);
        fp.write(reinterpret_cast<char *>(&data_in.stage_face_filename), sizeof(char) * MAX_STAGES * FS_FACE_FILENAME_MAX);
        fp.close();

// -------------------------------------- NPCS -------------------------------------- //
        // file_npc game_npcs[FS_GAME_MAX_NPCS]
        filename = std::string(FILEPATH) + "game_npcs" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }

        fp.write(reinterpret_cast<char *>(&data_in.game_npcs), sizeof(file_npc) * FS_GAME_MAX_NPCS);
        fp.close();

// -------------------------------------- OBJECTS -------------------------------------- //
        // file_object objects[FS_GAME_MAX_NPCS];
        filename = std::string(FILEPATH) + "game_objects" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.objects), sizeof(file_object) * FS_GAME_MAX_NPCS);
        fp.close();


// -------------------------------------- PROJECTILES -------------------------------------- //
        // file_projectile projectiles[FS_MAX_PROJECTILES]
        filename = std::string(FILEPATH) + "game_projectiles" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.projectiles), sizeof(file_projectile) * FS_MAX_PROJECTILES);
        fp.close();


// -------------------------------------- WEAPONS -------------------------------------- //
        // file_weapon weapons[FS_MAX_WEAPONS]
        filename = std::string(FILEPATH) + "game_weapons" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.weapons), sizeof(file_weapon) * FS_MAX_WEAPONS);
        fp.close();


// -------------------------------------- WEAPONS -------------------------------------- //
        // file_player players[FS_MAX_PLAYERS]
        filename = std::string(FILEPATH) + "game_players" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }

        fp.write(reinterpret_cast<char *>(&data_in.players), sizeof(file_player) * FS_MAX_PLAYERS);
        fp.close();



// -------------------------------------- ARTIFICIAL INTELIGENCE -------------------------------------- //
        // file_artificial_inteligence ai_types[FS_MAX_AI_TYPES]
        filename = std::string(FILEPATH) + "game_ai" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.ai_types), sizeof(file_artificial_inteligence) * FS_MAX_AI_TYPES);
        fp.close();


// -------------------------------------- TROPHIES -------------------------------------- //
        // st_file_trophy trophies[TROPHIES_MAX]
        filename = std::string(FILEPATH) + "game_trophies" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.trophies), sizeof(st_file_trophy) * TROPHIES_MAX);
        fp.close();


// -------------------------------------- SHOP -------------------------------------- //
        // st_shop_dialog shop_dialog_welcome
        // st_shop_dialog shop_dialog_goodbye
        filename = std::string(FILEPATH) + "game_shop" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.shop_dialog_welcome), sizeof(st_shop_dialog));
        fp.write(reinterpret_cast<char *>(&data_in.shop_dialog_goodbye), sizeof(st_shop_dialog));
        fp.close();


// -------------------------------------- ARMOR PIECES -------------------------------------- //
        // st_armor_piece armor_pieces[FS_PLAYER_ARMOR_PIECES_MAX]
        filename = std::string(FILEPATH) + "game_armorPieces" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.armor_pieces), sizeof(st_armor_piece) * FS_PLAYER_ARMOR_PIECES_MAX);
        fp.close();


// -------------------------------------- ANIM_TILES -------------------------------------- //
        // st_anim_map_tile anim_tiles[FS_ANIM_TILES_MAX]
        filename = std::string(FILEPATH) + "game_animTiles" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_game - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        }
        fp.write(reinterpret_cast<char *>(&data_in.anim_tiles), sizeof(st_anim_map_tile) * FS_ANIM_TILES_MAX);
        fp.close();






    }




    void file_io::write_all_stages(format_v4::file_stages &stages_data_in) const
    {
        std::ofstream fp;
        std::string filename = std::string(FILEPATH) + "stages" + sufix + ".dat";
        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::write_all_stages - could not write to file '" << filename << "'. Will create new one." << std::endl;
            fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
        } else {
            std::cout << "fio::write_game - recorded to file '" << filename << std::endl;
        }
        fp.write(reinterpret_cast<char *>(&stages_data_in), sizeof(struct format_v4::file_stages));
        fp.close();
    }



    void file_io::read_game(format_v4::file_game& data_out) const {
        FILE *fp;
        std::string filename = "";


        bool USE_NEW_READ = true;
        if (USE_NEW_READ == false) {
            filename = std::string(FILEPATH) + "game" + sufix + ".dat";
            fp = fopen(filename.c_str(), "rb");
            if (!fp) {
                std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
                fflush(stdout);
                return;
            }
            int read_result = fread(&data_out, sizeof(struct format_v4::file_game), 1, fp);
            if (read_result  == -1) {
                std::cout << ">>file_io::read_game - Error reading struct data from game file '" << filename << "'." << std::endl;
                fflush(stdout);
                exit(-1);
            }
            fclose(fp);
            return;
        }
// -------------------------------------- GAME -------------------------------------- //
        // float version;
        // char name[FS_CHAR_NAME_SIZE];
        // Sint8 semi_charged_projectile_id;
        // Sint8 player_items[FS_PLATER_ITEMS_N];
        // char stage_face_filename[MAX_STAGES][FS_FACE_FILENAME_MAX]
        filename = std::string(FILEPATH) + "game_properties" + sufix + ".dat";
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }


        if (unsigned int res = fread(&data_out.version, sizeof(float), 1, fp) != 1) {
            std::cout << ">>file_io::read_game - res: " << res << ", sizeof(float): " << sizeof(float) << ", Error reading struct data [version2] from game file '" << filename << "'." << std::endl;
            //exit(-1);
        }
        if (unsigned int res = fread(&data_out.name, sizeof(char), FS_CHAR_NAME_SIZE, fp) != FS_CHAR_NAME_SIZE) {
            std::cout << ">>file_io::read_game - res: " << res << ", sizeof(char): " << sizeof(char) << ", Error reading struct data from [name] game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        if (fread(&data_out.semi_charged_projectile_id, sizeof(Sint8), 1, fp) != 1) {
            std::cout << ">>file_io::read_game - Error reading struct data [semi_charged_projectile_id] from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        if (fread(&data_out.player_items, sizeof(Sint8), FS_PLATER_ITEMS_N, fp) != FS_PLATER_ITEMS_N) {
            std::cout << ">>file_io::read_game - Error reading struct data [player_items] from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        if (unsigned int res = fread(&data_out.stage_face_filename, sizeof(char), (MAX_STAGES*FS_FACE_FILENAME_MAX), fp) != MAX_STAGES*FS_FACE_FILENAME_MAX) {
            std::cout << ">>file_io::read_game res: - Error reading struct data [stage_face_filename] from game file '" << filename << "'." << std::endl;
            exit(-1);
        }

        fclose(fp);

// -------------------------------------- NPCS -------------------------------------- //
        // file_npc game_npcs[FS_GAME_MAX_NPCS]
        filename = std::string(FILEPATH) + "game_npcs" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);

        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.game_npcs, sizeof(file_npc), FS_GAME_MAX_NPCS, fp) != FS_GAME_MAX_NPCS) {
            std::cout << ">>file_io::read_game[game_npcs2] res: - Error reading data from game file '" << filename << "'." << std::endl;
            //exit(-1);
        }
        fclose(fp);

// -------------------------------------- OBJECTS -------------------------------------- //
        // file_object objects[FS_GAME_MAX_NPCS];
        filename = std::string(FILEPATH) + "game_objects" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.objects, sizeof(file_object), FS_GAME_MAX_NPCS, fp) != FS_GAME_MAX_NPCS) {
            std::cout << ">>file_io::read_game[objects] - Error reading data from game file '" << filename << "'." << std::endl;
            //exit(-1);
        }
        fclose(fp);


// -------------------------------------- PROJECTILES -------------------------------------- //
        // file_projectile projectiles[FS_MAX_PROJECTILES]
        filename = std::string(FILEPATH) + "game_projectiles" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.projectiles, sizeof(file_projectile), FS_MAX_PROJECTILES, fp) != FS_MAX_PROJECTILES) {
            std::cout << ">>file_io::read_game[projectiles2] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- WEAPONS -------------------------------------- //
        // file_weapon weapons[FS_MAX_WEAPONS]
        filename = std::string(FILEPATH) + "game_weapons" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.weapons, sizeof(file_weapon), FS_MAX_WEAPONS, fp) != FS_MAX_WEAPONS) {
            std::cout << ">>file_io::read_game[weapons] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- PLAYERS -------------------------------------- //
        // file_player players[FS_MAX_PLAYERS]
        filename = std::string(FILEPATH) + "game_players" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.players, sizeof(file_player), FS_MAX_PLAYERS, fp) != FS_MAX_PLAYERS) {
            std::cout << ">>file_io::read_game[players] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);



// -------------------------------------- ARTIFICIAL INTELIGENCE -------------------------------------- //
        // file_artificial_inteligence ai_types[FS_MAX_AI_TYPES]
        filename = std::string(FILEPATH) + "game_ai" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.ai_types, sizeof(file_artificial_inteligence), FS_MAX_AI_TYPES, fp) != FS_MAX_AI_TYPES) {
            std::cout << ">>file_io::read_game[ai_types] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- TROPHIES -------------------------------------- //
        // st_file_trophy trophies[TROPHIES_MAX]
        filename = std::string(FILEPATH) + "game_trophies" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.trophies, sizeof(st_file_trophy), TROPHIES_MAX, fp) != TROPHIES_MAX) {
            std::cout << ">>file_io::read_game[trophies] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- SHOP -------------------------------------- //
        // st_shop_dialog shop_dialog_welcome
        // st_shop_dialog shop_dialog_goodbye
        filename = std::string(FILEPATH) + "game_shop" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        int read_result1 = fread(&data_out.shop_dialog_welcome, sizeof(st_shop_dialog), 1, fp);
        if (read_result1  == -1) {
            std::cout << ">>file_io::read_game[shop_dialog_welcome] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        int read_result2 = fread(&data_out.shop_dialog_goodbye, sizeof(st_shop_dialog), 1, fp);
        if (read_result2  == -1) {
            std::cout << ">>file_io::read_game[shop_dialog_goodbye] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- ARMOR PIECES -------------------------------------- //
        // st_armor_piece armor_pieces[FS_PLAYER_ARMOR_PIECES_MAX]
        filename = std::string(FILEPATH) + "game_armorPieces" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.armor_pieces, sizeof(st_armor_piece), FS_PLAYER_ARMOR_PIECES_MAX, fp) != FS_PLAYER_ARMOR_PIECES_MAX) {
            std::cout << ">>file_io::read_game[armor_pieces] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


// -------------------------------------- ANIM_TILES -------------------------------------- //
        // st_anim_map_tile anim_tiles[FS_ANIM_TILES_MAX]
        filename = std::string(FILEPATH) + "game_animTiles" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << ">>file_io::read_game - file '" << filename << "' not found." << std::endl;
            return;
        }
        if (fread(&data_out.anim_tiles, sizeof(st_anim_map_tile), FS_ANIM_TILES_MAX, fp) != FS_ANIM_TILES_MAX) {
            std::cout << ">>file_io::read_game[anim_tiles] - Error reading data from game file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fclose(fp);


    }


    void file_io::read_all_stages(format_v4::file_stages& stages_data_out)
    {
        std::ifstream fp;
        std::string filename = std::string(FILEPATH) + "stages" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
            return;
        }
        fp.read(reinterpret_cast<char *>(&stages_data_out), sizeof(struct format_v4::file_stages));
        fp.close();
    }

    void file_io::read_stage(format_v4::file_stage &stages_data_out, short stage_n)
    {
        FILE *fp;
        std::string filename = std::string(FILEPATH) + "stages" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            printf("ERROR.read_stage: Could not read stage '%s'\n", filename.c_str());
            fflush(stdout);
            return;
        }
        fseek(fp, sizeof(format_v4::file_stage) * stage_n, SEEK_SET);
        int read_result = fread(&stages_data_out, sizeof(struct format_v4::file_stage), 1, fp);
        if (read_result == -1) {
            printf(">>file_io::read_game - Error reading struct data from stage file.\n");
            fflush(stdout);
            exit(-1);
        }
        fclose(fp);
    }

    bool file_io::file_exists(std::string filename) const
    {
        bool res = false;
        FILE *fp;
        fp = fopen(filename.c_str(), "rb");
        if (fp) {
            res = true;
            fclose(fp);
        }
        return res;
    }

    bool file_io::directory_exists(std::string filename) const
    {

#ifndef PLAYSTATION2
        struct stat info;
        if (stat( filename.c_str(), &info) != 0) {
            printf( "cannot access %s\n", filename.c_str() );
            return false;
        } else if (info.st_mode & S_IFDIR) { // S_ISDIR() doesn't exist on my windows
            printf( "%s is a directory\n", filename.c_str() );
            return true;
        } else {
            printf( "%s is no directory\n", filename.c_str() );
            return false;
        }
#else
        return false;
#endif
    }

    std::vector<std::string> file_io::read_game_list() const
    {
        std::string filename = GAMEPATH + "/games/";
        filename = StringUtils::clean_filename(filename);
        return read_directory_list(filename, true);
    }

    std::vector<std::string> file_io::read_directory_list(std::string filename, bool dir_only) const
    {
        std::vector<std::string> res;
        filename = StringUtils::clean_filename(filename);

        // check if games folder exists
        if (!directory_exists(filename)) {
            std::cout << "ERROR: games folder not found." << std::endl;
            return res;
        }

#ifndef PLAYSTATION2

        DIR *dir = opendir(filename.c_str());
        struct dirent *entry = readdir(dir);
        while (entry != NULL) {
            std::string dir_name = std::string(entry->d_name);
            if (dir_name != "." && dir_name != "..") {
                DIR *child_dir;
                std::string child_dir_path = filename + std::string("/") + dir_name;
                child_dir = opendir (child_dir_path.c_str());

                if (dir_only == true && child_dir != NULL) {
                    res.push_back(dir_name);
                } else if (dir_only == false && child_dir == NULL) {
                    res.push_back(dir_name);
                }
            }
            entry = readdir(dir);

        }
        closedir(dir);
#endif
        return res;
    }

    std::vector<std::string> file_io::read_file_list(std::string filename) const
    {
        return read_directory_list(filename, false);
    }





    bool file_io::save_exists() const
    {
        std::string filename = std::string(SAVEPATH) + std::string("/") + GAMENAME + std::string(".sav");
        filename = StringUtils::clean_filename(filename);
        std::ifstream fp(filename.c_str());
        if (fp.good()) {
            return true;
        }
        return false;
    }

    void file_io::load_config(format_v4::st_game_config& config)
    {
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + "/config.sav";
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << "WARNING: Could not read config file '" << filename.c_str() << "'." << std::endl;
        } else {
            int read_result = fread(&config, sizeof(struct format_v4::st_game_config), 1, fp);
            if (read_result  == -1) {
                printf(">>file_io::read_game - Error reading struct data from game file '%s'.\n", filename.c_str());
                fflush(stdout);
                exit(-1);
            }
            fclose(fp);
        }
        if (config.get_current_platform() != config.platform) {
            config.reset();
        }
#ifndef PC
        config.video_filter = VIDEO_FILTER_NOSCALE;
        std::cout << "IO::load_config - SET video_filter to " << VIDEO_FILTER_NOSCALE << ", value: " << config.video_filter << std::endl;
#endif
        // ### DEBUG ### //
        //config.game_finished = true;
    }

    void file_io::save_config(st_game_config &config) const
    {
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + "/config.sav";
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "wb");
        if (!fp) {
            std::cout << "Error: Could not open config file '" << filename << "'." << std::endl;
            exit(-1);
        }
        fwrite(&config, sizeof(struct format_v4::st_game_config), 1, fp);
        fclose(fp);
    }

    void file_io::read_save(format_v4::st_save& data_out) const
    {
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + std::string("/") + GAMENAME + std::string(".sav");
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cout << "ERROR: Could not read save" << std::endl;
            exit(-1);
        }
        int read_result = fread(&data_out, sizeof(struct format_v4::st_save), 1, fp);
        if (read_result  == -1) {
            printf(">>file_io::read_game - Error reading struct data from game file '%s'.\n", filename.c_str());
            fflush(stdout);
            exit(-1);
        }


        /*
        if (GAME_FLAGS[FLAG_PLAYER_ROCKBOT]) {
            data_out.selected_player = PLAYER_ROCKBOT;
        } else if (GAME_FLAGS[FLAG_PLAYER_BETABOT]) {
            data_out.selected_player = PLAYER_BETABOT;
        } else if (GAME_FLAGS[FLAG_PLAYER_CANDYBOT]) {
            data_out.selected_player = PLAYER_CANDYBOT;
        } else if (GAME_FLAGS[FLAG_PLAYER_KITTYBOT]) {
            data_out.selected_player = PLAYER_KITTYBOT;
        }
        */




        // ------- DEBUG ------- //
        /*
        data_out.stages[INTRO_STAGE] = 1;
        for (int i=1; i<SKULLCASTLE5; i++) {
            data_out.stages[i] = 1;
        }
        */
        //data_out.stages[MAGEBOT] = 0;
        data_out.stages[SEAHORSEBOT] = 1;
        data_out.stages[DAISIEBOT] = 1;
        /*
        //data_out.stages[MUMMYBOT] = 1;
        //data_out.items.lifes = 0;

        data_out.stages[DYNAMITEBOT] = 1;
        data_out.armor_pieces[ARMOR_ARMS] = true;
        data_out.armor_pieces[ARMOR_BODY] = true;
        data_out.armor_pieces[ARMOR_LEGS] = true;
        */

        // ------- DEBUG ------- //


        if (data_out.items.lifes > 9) {
            data_out.items.lifes = 3;
        }

        fclose(fp);
    }

    bool file_io::write_save(format_v4::st_save& data_in)
    {
        FILE *fp;
        std::string filename = std::string(SAVEPATH) + std::string("/") + GAMENAME + std::string(".sav");
        filename = StringUtils::clean_filename(filename);
        fp = fopen(filename.c_str(), "wb");
        if (!fp) {
            std::cout << "Error: Could not open save-file '" << filename << "'." << std::endl;
            return false;
        }
        fwrite(&data_in, sizeof(struct format_v4::st_save), 1, fp);
        fclose(fp);
        return true;
    }


    int file_io::read_stage_boss_id(Uint8 stage_n, file_stage &stages_data_out)
    {
        /// @NOTE: desabilitei essa função pois quando o formato de arquivo muda, causa um crash
        /// Isso provavelmente causará estouro de memória no PSP
        /// Então estou sobrescrevendo o próprio stage_data, passado como referência
        FILE *fp;

        std::string filename = std::string(FILEPATH) + "stages" + sufix + ".dat";
        filename = StringUtils::clean_filename(filename);

        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            printf("ERROR.read_stage: Could not read stage '%s'\n", filename.c_str());
            fflush(stdout);
            exit(-1);
        }
        fseek(fp, sizeof(format_v4::file_stage) * stage_n, SEEK_SET);
        int read_result = fread(&stages_data_out, sizeof(struct format_v4::file_stage), 1, fp);
        if (read_result == -1) {
            printf(">>file_io::read_game - Error reading struct data from stage file.\n");
            fflush(stdout);
            exit(-1);
        }
        fclose(fp);
        return stages_data_out.boss.id_npc;
    }




    // create and save empty files for new-game
    void file_io::generate_files()
    {
        CURRENT_FILE_FORMAT::file_game data_in;
        write_game(data_in);
        CURRENT_FILE_FORMAT::file_stages stages_data_in;
        write_all_stages(stages_data_in);
    }







    void file_io::load_scene_sequence(std::vector<file_scene_sequence> &scene_sequence)
    {
        scene_sequence.clear();
        file_scene_sequence temp;
        std::ifstream fp;
        std::string filename = std::string(FILEPATH) + "sequence.scn";
        filename = StringUtils::clean_filename(filename);

        fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
            return;
        }
        while(!fp.eof()) {
            fp.read(reinterpret_cast<char *>(&temp), sizeof(struct file_scene_sequence));
            scene_sequence.push_back(temp);
        }
        fp.close();
    }

    void file_io::save_scene_sequence(std::vector<file_scene_sequence> &scene_sequence)
    {
        std::ofstream fp;
        std::string filename = std::string(FILEPATH) + "sequence.scn";
        filename = StringUtils::clean_filename(filename);

        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
            return;
        }
        for (unsigned int i=0; i<scene_sequence.size(); i++) {
            fp.write(reinterpret_cast<char *>(&scene_sequence.at(i)), sizeof(struct file_scene_sequence));
        }
        fp.close();
    }


    void file_io::load_scenes(std::vector<file_scene> &scenes)
    {
        scenes.clear();
        file_scene temp;
        std::ifstream fp;
        std::string filename = std::string(FILEPATH) + "scenes.scn";
        filename = StringUtils::clean_filename(filename);

        fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
            return;
        }
        while(!fp.eof()) {
            fp.read(reinterpret_cast<char *>(&temp), sizeof(struct file_scene));
            scenes.push_back(temp);
        }
        fp.close();
    }


    void file_io::save_scenes(std::vector<file_scene> &scenes)
    {
        std::ofstream fp;
        std::string filename = std::string(FILEPATH) + "scenes.scn";
        filename = StringUtils::clean_filename(filename);

        fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
        if (!fp.is_open()) {
            std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
            return;
        }
        for (unsigned int i=0; i<scenes.size(); i++) {
            fp.write(reinterpret_cast<char *>(&scenes.at(i)), sizeof(struct file_scene));
        }
        fp.close();
    }




}

