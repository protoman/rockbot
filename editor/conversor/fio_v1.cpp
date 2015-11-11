#include "fio_v1.h"


extern std::string FILEPATH;
extern std::string SAVEPATH;

fio_v1::fio_v1()
{
    sufix = "_v301";
}


void fio_v1::read_game(v1_file_game& data_out) const {
    FILE *fp;
    std::string filename = "";


// -------------------------------------- GAME -------------------------------------- //
    // float version;
    // char name[FS_CHAR_NAME_SIZE];
    // Sint8 semi_charged_projectile_id;
    // Sint8 player_items[FS_PLATER_ITEMS_N];
    // char stage_face_filename[MAX_STAGES][FS_FACE_FILENAME_MAX]
    filename = std::string(FILEPATH) + "data/game_properties" + sufix + ".dat";
    fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        std::cout << ">>fio_v1::read_game - file '" << filename << "' not found." << std::endl;
        return;
    }


    if (fread(&data_out.version, sizeof(float), 1, fp)  == -1) {
        std::cout << ">>fio_v1::read_game - Error reading struct data [version] from game file '" << filename << "'." << std::endl;
        exit(-1);
    }
    if (fread(&data_out.name, sizeof(char), V1_FS_CHAR_NAME_SIZE, fp)  == -1) {
        std::cout << ">>fio_v1::read_game - Error reading struct data from [name] game file '" << filename << "'." << std::endl;
        exit(-1);
    }
    if (fread(&data_out.semi_charged_projectile_id, sizeof(Sint8), 1, fp)  == -1) {
        std::cout << ">>fio_v1::read_game - Error reading struct data [semi_charged_projectile_id] from game file '" << filename << "'." << std::endl;
        exit(-1);
    }
    if (fread(&data_out.player_items, sizeof(Sint8), V1_FS_PLATER_ITEMS_N, fp)  == -1) {
        std::cout << ">>fio_v1::read_game - Error reading struct data [player_items] from game file '" << filename << "'." << std::endl;
        exit(-1);
    }
    if (fread(&data_out.stage_face_filename, sizeof(char) * V1_MAX_STAGES * V1_FS_FACE_FILENAME_MAX, 1, fp)  == -1) {
        std::cout << ">>fio_v1::read_game - Error reading struct data [stage_face_filename] from game file '" << filename << "'." << std::endl;
        exit(-1);
    }


    std::cout << "version: " << data_out.version << ", name: '" << data_out.name << "', semi_charged_projectile_id: " << (int)data_out.semi_charged_projectile_id << std::endl;

    fclose(fp);

// -------------------------------------- NPCS -------------------------------------- //
    // file_npc game_npcs[FS_GAME_MAX_NPCS]
    filename = std::string(FILEPATH) + "data/game_npcs" + sufix + ".dat";
    fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        std::cout << ">>fio_v1::read_game - file '" << filename << "' not found." << std::endl;
        return;
    }
    if (fread(&data_out.game_npcs, sizeof(v1_file_npc)*V1_FS_GAME_MAX_NPCS, 1, fp)  == -1) {
        std::cout << ">>fio_v1::read_game - Error reading data from game file '" << filename << "'." << std::endl;
        exit(-1);
    }
    fclose(fp);

// -------------------------------------- OBJECTS -------------------------------------- //
    // file_object objects[FS_GAME_MAX_NPCS];
    filename = std::string(FILEPATH) + "data/game_objects" + sufix + ".dat";
    fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        std::cout << ">>fio_v1::read_game - file '" << filename << "' not found." << std::endl;
        return;
    }
    if (fread(&data_out.objects, sizeof(v1_file_object)*V1_FS_GAME_MAX_NPCS, 1, fp)  == -1) {
        std::cout << ">>fio_v1::read_game - Error reading data from game file '" << filename << "'." << std::endl;
        exit(-1);
    }
    fclose(fp);


// -------------------------------------- PROJECTILES -------------------------------------- //
    // file_projectile projectiles[FS_MAX_PROJECTILES]
    filename = std::string(FILEPATH) + "data/game_projectiles" + sufix + ".dat";
    fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        std::cout << ">>fio_v1::read_game - file '" << filename << "' not found." << std::endl;
        return;
    }
    if (fread(&data_out.projectiles, sizeof(v1_file_projectile)*V1_FS_MAX_PROJECTILES, 1, fp)  == -1) {
        std::cout << ">>fio_v1::read_game - Error reading data from game file '" << filename << "'." << std::endl;
        exit(-1);
    }
    fclose(fp);


// -------------------------------------- WEAPONS -------------------------------------- //
    // file_weapon weapons[FS_MAX_WEAPONS]
    filename = std::string(FILEPATH) + "data/game_weapons" + sufix + ".dat";
    fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        std::cout << ">>fio_v1::read_game - file '" << filename << "' not found." << std::endl;
        return;
    }
    if (fread(&data_out.weapons, sizeof(v1_file_weapon)*V1_FS_MAX_WEAPONS, 1, fp)  == -1) {
        std::cout << ">>fio_v1::read_game - Error reading data from game file '" << filename << "'." << std::endl;
        exit(-1);
    }
    fclose(fp);


// -------------------------------------- PLAYERS -------------------------------------- //
    // file_player players[FS_MAX_PLAYERS]
    filename = std::string(FILEPATH) + "data/game_players" + sufix + ".dat";
    fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        std::cout << ">>fio_v1::read_game - file '" << filename << "' not found." << std::endl;
        return;
    }
    if (fread(&data_out.players, sizeof(v1_file_player)*V1_FS_MAX_PLAYERS, 1, fp)  == -1) {
        std::cout << ">>fio_v1::read_game - Error reading data from game file '" << filename << "'." << std::endl;
        exit(-1);
    }
    fclose(fp);



// -------------------------------------- ARTIFICIAL INTELIGENCE -------------------------------------- //
    // file_artificial_inteligence ai_types[FS_MAX_AI_TYPES]
    filename = std::string(FILEPATH) + "data/game_ai" + sufix + ".dat";
    fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        std::cout << ">>fio_v1::read_game - file '" << filename << "' not found." << std::endl;
        return;
    }
    if (fread(&data_out.ai_types, sizeof(v1_file_artificial_inteligence)*V1_FS_MAX_AI_TYPES, 1, fp)  == -1) {
        std::cout << ">>fio_v1::read_game - Error reading data from game file '" << filename << "'." << std::endl;
        exit(-1);
    }
    fclose(fp);


// -------------------------------------- TROPHIES -------------------------------------- //
    // st_file_trophy trophies[TROPHIES_MAX]
    filename = std::string(FILEPATH) + "data/game_trophies" + sufix + ".dat";
    fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        std::cout << ">>fio_v1::read_game - file '" << filename << "' not found." << std::endl;
        return;
    }
    if (fread(&data_out.trophies, sizeof(v1_st_file_trophy)*V1_TROPHIES_MAX, 1, fp)  == -1) {
        std::cout << ">>fio_v1::read_game - Error reading data from game file '" << filename << "'." << std::endl;
        exit(-1);
    }
    fclose(fp);


// -------------------------------------- SHOP -------------------------------------- //
    // st_shop_dialog shop_dialog_welcome
    // st_shop_dialog shop_dialog_goodbye
    filename = std::string(FILEPATH) + "data/game_shop" + sufix + ".dat";
    fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        std::cout << ">>fio_v1::read_game - file '" << filename << "' not found." << std::endl;
        return;
    }
    int read_result1 = fread(&data_out.shop_dialog_welcome, sizeof(v1_st_shop_dialog), 1, fp);
    if (read_result1  == -1) {
        std::cout << ">>fio_v1::read_game - Error reading data from game file '" << filename << "'." << std::endl;
        exit(-1);
    }
    int read_result2 = fread(&data_out.shop_dialog_goodbye, sizeof(v1_st_shop_dialog), 1, fp);
    if (read_result2  == -1) {
        std::cout << ">>fio_v1::read_game - Error reading data from game file '" << filename << "'." << std::endl;
        exit(-1);
    }
    fclose(fp);


// -------------------------------------- ARMOR PIECES -------------------------------------- //
    // st_armor_piece armor_pieces[FS_PLAYER_ARMOR_PIECES_MAX]
    filename = std::string(FILEPATH) + "data/game_armorPieces" + sufix + ".dat";
    fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        std::cout << ">>fio_v1::read_game - file '" << filename << "' not found." << std::endl;
        return;
    }
    if (fread(&data_out.armor_pieces, sizeof(v1_st_armor_piece)*V1_FS_PLAYER_ARMOR_PIECES_MAX, 1, fp)  == -1) {
        std::cout << ">>fio_v1::read_game - Error reading data from game file '" << filename << "'." << std::endl;
        exit(-1);
    }
    fclose(fp);


// -------------------------------------- ANIM_TILES -------------------------------------- //
    // st_anim_map_tile anim_tiles[FS_ANIM_TILES_MAX]
    filename = std::string(FILEPATH) + "data/game_animTiles" + sufix + ".dat";
    fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        std::cout << ">>fio_v1::read_game - file '" << filename << "' not found." << std::endl;
        return;
    }
    if (fread(&data_out.anim_tiles, sizeof(v1_st_anim_map_tile)*V1_FS_ANIM_TILES_MAX, 1, fp)  == -1) {
        std::cout << ">>fio_v1::read_game - Error reading data from game file '" << filename << "'." << std::endl;
        exit(-1);
    }
    fclose(fp);


}


void fio_v1::read_all_stages(v1_file_stages& stages_data_out)
{
    std::ifstream fp;
    std::string filename = std::string(FILEPATH) + "data/stages" + sufix + ".dat";
    fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
    if (!fp.is_open()) {
        std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
        return;
    }
    fp.read(reinterpret_cast<char *>(&stages_data_out), sizeof(struct v1_file_stages));
    fp.close();
}

void fio_v1::read_stage(v1_file_stage &stages_data_out, short stage_n)
{
    FILE *fp;
    std::string filename = std::string(FILEPATH) + "data/stages" + sufix + ".dat";
    fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        printf("ERROR.read_stage: Could not read stage '%s'\n", filename.c_str());
        fflush(stdout);
        return;
    }
    fseek(fp, sizeof(v1_file_stage) * stage_n, SEEK_SET);
    int read_result = fread(&stages_data_out, sizeof(struct v1_file_stage), 1, fp);
    if (read_result == -1) {
        printf(">>fio_v1::read_game - Error reading struct data from stage file.\n");
        fflush(stdout);
        exit(-1);
    }
    fclose(fp);
}

bool fio_v1::file_exists(std::string filename) const
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




/*
bool fio_v1::save_exists() const
{
    std::string filename = std::string(SAVEPATH) + "/game_v301.sav";
    std::ifstream fp(filename.c_str());
    if (fp.good()) {
        return true;
    }
    return false;
}

void fio_v1::load_config(v1_st_game_config& config)
{
    FILE *fp;
    std::string filename = std::string(SAVEPATH) + "/config_v301.sav";
    fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        std::cout << "WARNING: Could not read config file '" << filename.c_str() << "'." << std::endl;
    } else {
        int read_result = fread(&config, sizeof(struct v1_st_game_config), 1, fp);
        if (read_result  == -1) {
            printf(">>fio_v1::read_game - Error reading struct data from game file '%s'.\n", filename.c_str());
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

void fio_v1::save_config(st_game_config &config) const
{
    FILE *fp;
    std::string filename = std::string(SAVEPATH) + "/config_v301.sav";
    fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        std::cout << "Error: Could not open config file '" << filename << "'." << std::endl;
        exit(-1);
    }
    fwrite(&config, sizeof(struct v1_st_game_config), 1, fp);
    fclose(fp);
}

void fio_v1::read_save(v1_st_save& data_out) const
{
    FILE *fp;
    std::string filename = std::string(SAVEPATH) + "/game_v301.sav";
    fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        std::cout << "ERROR: Could not read save" << std::endl;
        exit(-1);
    }
    int read_result = fread(&data_out, sizeof(struct v1_st_save), 1, fp);
    if (read_result  == -1) {
        printf(">>fio_v1::read_game - Error reading struct data from game file '%s'.\n", filename.c_str());
        fflush(stdout);
        exit(-1);
    }


    if (GAME_FLAGS[FLAG_PLAYER_ROCKBOT]) {
        data_out.selected_player = PLAYER_ROCKBOT;
    } else if (GAME_FLAGS[FLAG_PLAYER_BETABOT]) {
        data_out.selected_player = PLAYER_BETABOT;
    } else if (GAME_FLAGS[FLAG_PLAYER_CANDYBOT]) {
        data_out.selected_player = PLAYER_CANDYBOT;
    } else if (GAME_FLAGS[FLAG_PLAYER_KITTYBOT]) {
        data_out.selected_player = PLAYER_KITTYBOT;
    }





    data_out.armor_pieces[ARMOR_ARMS] = true;
    data_out.armor_pieces[ARMOR_BODY] = true;
    data_out.armor_pieces[ARMOR_LEGS] = true;

    // ------- DEBUG ------- //


    if (data_out.items.lifes > 9) {
        data_out.items.lifes = 3;
    }

    fclose(fp);
}

bool fio_v1::write_save(v1_st_save& data_in)
{
    FILE *fp;
    std::string filename = std::string(SAVEPATH) + "/game_v301.sav";
    fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        std::cout << "Error: Could not open save-file '" << filename << "'." << std::endl;
        return false;
    }
    fwrite(&data_in, sizeof(struct v1_st_save), 1, fp);
    fclose(fp);
    return true;
}
*/

int fio_v1::read_stage_boss_id(int stage_n, v1_file_stage &stages_data_out)
{
    FILE *fp;

    std::string filename = std::string(FILEPATH) + "data/stages" + sufix + ".dat";
    fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        printf("ERROR.read_stage: Could not read stage '%s'\n", filename.c_str());
        fflush(stdout);
        exit(-1);
    }
    fseek(fp, sizeof(v1_file_stage) * stage_n, SEEK_SET);
    int read_result = fread(&stages_data_out, sizeof(struct v1_file_stage), 1, fp);
    if (read_result == -1) {
        printf(">>fio_v1::read_game - Error reading struct data from stage file.\n");
        fflush(stdout);
        exit(-1);
    }
    fclose(fp);
    return stages_data_out.boss.id_npc;
}

void fio_v1::read_colormap(SDL_Color (&colormap)[COLOR_COUNT])
{
    int line_number = 0;
    FILE *fp;
    std::string filename = std::string(FILEPATH) + "data/rockbot.gpl";

    fp = fopen(filename.c_str(), "rb");

    if (!fp) {
        printf("ERROR.read_stage: Could not read stage '%s'\n", filename.c_str());
        std::fflush(stdout);
        exit(-1);
    }

    char line[128];
    int n = 0;                                      // used to ignore first 4 lines
    while (fgets(line, sizeof line, fp) != NULL) {  // get a line from text
        //std::cout << ">>>>> fio_v1::read_colormap - line: " << line << std::endl;
        if (n <= 3) {
            n++;
            continue;
        }
        // now split the string by its spaces
        std::vector<std::string> splited_line = split(std::string(line), std::string(" "));
        std::vector<std::string>::iterator it;
        int i = 0;
        for (it=splited_line.begin(); it!=splited_line.end(); it++) {
            //std::cout << "'" << (*it) << "', ";

            int number = 0;
            std::istringstream is((*it));
            is >> number;


            if (i == 0) {
                colormap[line_number].r = number;
            } else if (i == 1) {
                colormap[line_number].g = number;
            } else if (i == 2) {
                colormap[line_number].b = number;
            } else if (i > 2) { // limit to the first 3 elements
                break;
            }
            //std::cout << "'" << number << "', ";
            i++;
        }
        //std::cout << std::endl;

        line_number++;
        n++;
    }


}


std::vector<std::string> fio_v1::split(std::string str, std::string sep)
{
    char* cstr = const_cast<char*>(str.c_str());
    char* current;
    std::vector<std::string> arr;
    current = strtok(cstr, sep.c_str());
    while(current != NULL){
        arr.push_back(current);
        current=strtok(NULL, sep.c_str());
    }
    return arr;
}




