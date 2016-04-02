#include "fio_strings.h"

#include <iostream>
#include <fstream>

#include "defines.h"
#include "aux_tools/stringutils.h"
#include "file/fio_common.h"

extern std::string FILEPATH;
extern std::string GAMEPATH;

#define STRINGS_INGAME_FILENAME "/strings_ingame.dat"
#define STRINGS_COMMON_FILENAME "/common_strings.dat"

fio_strings::fio_strings()
{
}


namespace format_v4 {

    fio_strings::fio_strings()
    {
        _dialogs_stage_id = -1;
    }

    std::string fio_strings::get_ingame_string(int n)
    {
        if (string_list.size() == 0) {
            string_list = load_game_strings();
        }
        if (n < 0 || n > string_list.size()) {
            return "";
        }

        //std::cout << "FIO_STRINGS::get_ingame_string[" << n << "]: " << string_list.at(n) << std::endl;

        return string_list.at(n);
    }


    std::vector<std::string> fio_strings::load_game_strings()
    {
        return load_game_strings_from_file(get_game_strings_filename());
    }

    std::vector<std::string> fio_strings::load_game_strings_from_file(std::string filename)
    {
        std::vector<std::string> res;
        filename = StringUtils::clean_filename(filename);
        std::ifstream fp(filename.c_str());

        if (!fp.is_open()) {
            std::cout << "[WARNING] file_io::load_game_strings - file '" << filename << "' not found, will generate default..." << std::endl;
            if (filename == get_game_strings_filename()) {
                create_default_ingame_strings();
            } else if (filename == get_common_strings_filename()) {
                create_default_common_strings();
            }
            fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
            if (!fp.is_open()) {
                std::cout << "[WARNING] file_io::load_game_strings - Critical error, can't open' '" << filename << "' for reading." << std::endl;
            }
        }

        std::string str;
        while (getline(fp, str)) {
            StringUtils::replace_all(str, "\n", "");
            StringUtils::replace_all(str, "\r", "");
            res.push_back(str);
        }
        fp.close();

        if (res.size() == 0) {
            if (filename == get_game_strings_filename()) {
                create_default_ingame_strings();
            } else if (filename == get_common_strings_filename()) {
                create_default_common_strings();
            }
        } else if (filename == get_game_strings_filename() && res.size() < strings_ingame_COUNT) {
            res = add_missing_default_ingame_strings(res);
        }

        return res;
    }

    void fio_strings::save_game_strings(std::vector<std::string> list, std::string filename)
    {
        std::ofstream fp(filename.c_str());
        if (!fp.is_open()) {
            std::cout << ">> fio_strings::create_default_ingame_strings: Could not open '" << filename << "' for writting." << std::endl;
            return;
        }

        for (int i=0; i<list.size(); i++) {
            std::string line = list.at(i);
            std::cout << "save_game_strings - add line '" << line << "'" << std::endl;
            fp << line.c_str();
        }
        fp.close();
    }

    std::string fio_strings::get_common_strings_filename()
    {
        std::string filename = FILEPATH + STRINGS_COMMON_FILENAME;
        filename = StringUtils::clean_filename(filename);
        return filename;
    }

    std::string fio_strings::get_game_strings_filename()
    {
        std::string filename = std::string(GAMEPATH) + "/shared/" + STRINGS_INGAME_FILENAME;
        filename = StringUtils::clean_filename(filename);
        return filename;
    }


    bool format_v4::fio_strings::file_exists(std::string filename) const
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

    std::vector<std::string> fio_strings::get_default_ingame_strings_list()
    {
        char lines[strings_ingame_COUNT][STRINGS_LINE_SIZE];

        sprintf(lines[strings_ingame_newgame], "%s", "NEW GAME");
        sprintf(lines[strings_ingame_loadgame], "%s", "LOAD GAME");
        sprintf(lines[strings_ingame_config], "%s", "CONFIG");
        sprintf(lines[strings_ingame_about], "%s", "ABOUT");
        sprintf(lines[strings_ingame_password], "%s", "PASSWORD");
        sprintf(lines[strings_ingame_gameover], "%s", "GAME OVER");
        sprintf(lines[strings_ingame_and], "%s", "AND");
        sprintf(lines[strings_ingame_selectgame], "%s", "SELECT GAME");
        sprintf(lines[strings_ingame_savegameerror1], "%s", "ERROR WHILE SAVING GAME,");
        sprintf(lines[strings_ingame_savegameerror2], "%s", "PLEASE CHECK THAT THE DEVICE OR");
        sprintf(lines[strings_ingame_savegameerror3], "%s", "FILE IS NOT WRITE-PROTECTED.");
        sprintf(lines[strings_ingame_copyrightline], "%s", "\xA9 2009-2016 UPPERLAND STUDIOS");
        //sprintf(lines[strings_ingame_copyrightline], "%s", "(c) 2009-2016 UPPERLAND STUDIOS");
        sprintf(lines[strings_ingame_audio], "%s", "AUDIO");
        sprintf(lines[strings_ingame_input], "%s", "INPUT");
        sprintf(lines[strings_ingame_video], "%s", "VIDEO");
        sprintf(lines[strings_ingame_leavestage], "%s", "LEAVE STAGE");
        sprintf(lines[strings_ingame_video_windowed], "%s", "WINDOWED");
        sprintf(lines[strings_ingame_video_fullscreen], "%s", "FULLSCREEN");
        sprintf(lines[strings_ingame_video_noscale], "%s", "NO SCALE  ");
        sprintf(lines[strings_ingame_video_size2x], "%s", "SIZE2X    ");
        sprintf(lines[strings_ingame_video_scale2x], "%s", "SCALE2X   ");
        sprintf(lines[strings_ingame_config_restart1], "%s", "PLEASE RESTART THE GAME");
        sprintf(lines[strings_ingame_config_restart2], "%s", "FOR THE CONFIGURATION");
        sprintf(lines[strings_ingame_config_restart3], "%s", "TO TAKE EFFECT");
        sprintf(lines[strings_ingame_config_presstorestart], "%s", "PRESS A BUTTON TO RETURN");
        sprintf(lines[strings_ingame_pressanykey], "%s", "PRESS ANY KEY OR BUTTON");
        sprintf(lines[strings_ingame_config_set], "%s", "SET");
        sprintf(lines[strings_ingame_config_key_up], "%s", "UP");
        sprintf(lines[strings_ingame_config_key_down], "%s", "DOWN");
        sprintf(lines[strings_ingame_config_key_left], "%s", "LEFT");
        sprintf(lines[strings_ingame_config_key_right], "%s", "RIGHT");
        sprintf(lines[strings_ingame_config_key_jump], "%s", "JUMP");
        sprintf(lines[strings_ingame_config_key_dash], "%s", "DASH");
        sprintf(lines[strings_ingame_config_key_fire], "%s", "FIRE");
        sprintf(lines[strings_ingame_config_key_start], "%s", "START");
        sprintf(lines[strings_ingame_config_key_keyl], "%s", "BUTTON-L");
        sprintf(lines[strings_ingame_config_key_keyr], "%s", "BUTTON-R");
        sprintf(lines[strings_ingame_config_key_pressnew], "%s", "PRESS NEW KEY/BUTTON");
        sprintf(lines[strings_ingame_config_key_reset], "%s", "RESET TO DEFAULT");
        sprintf(lines[strings_ingame_config_key_directional], "%s", "DIRECTIONAL");
        sprintf(lines[strings_ingame_config_key_directional_analog], "%s", "ANALOG");
        sprintf(lines[strings_ingame_config_key_directional_digital], "%s", "DIGITAL");
        sprintf(lines[strings_ingame_passwordinvalid], "%s", "PASSWORD INVALID");
        sprintf(lines[strings_ingame_pressstart], "%s", "PRESS START");
        sprintf(lines[strings_ingame_engineerror], "%s", "ROCKBOT ENGINE ERROR");
        sprintf(lines[strings_ingame_nogames], "%s", "NO GAMES AVAILABLE");
        sprintf(lines[strings_ingame_quitgame], "%s", "QUIT GAME?");
        sprintf(lines[strings_ingame_yes], "%s", "YES");
        sprintf(lines[strings_ingame_no], "%s", "NO");
        sprintf(lines[strings_ingame_life], "%s", "LIFE");
        sprintf(lines[strings_ingame_item], "%s", "ITEM");
        sprintf(lines[strings_ingame_gotarmor_type_ability], "%s", "GIVE YOU THE ABILITY TO");
        sprintf(lines[strings_ingame_gotarmor_type_arms], "%s", "THIS IMPROVED ARMS WILL");
        sprintf(lines[strings_ingame_gotarmor_type_arms_msg1], "%s", "FIRE ALWAYS CHARGED");
        sprintf(lines[strings_ingame_gotarmor_type_arms_msg2], "%s", "CHARGE A LASER BEAM");
        sprintf(lines[strings_ingame_gotarmor_type_arms_msg3], "%s", "FIRE A SUPER-SHOT!");
        sprintf(lines[strings_ingame_gotarmor_type_arms_msg4], "%s", "THROW A HADOUKEN");
        sprintf(lines[strings_ingame_gotarmor_type_legs], "%s", "THOSE LIGHTER LEGS");
        sprintf(lines[strings_ingame_gotarmor_type_legs_msg1], "%s", "DASH IN MIDDLE-AIR.");
        sprintf(lines[strings_ingame_gotarmor_type_legs_msg2], "%s", "EXECUTE DOUBLE JUMP");
        sprintf(lines[strings_ingame_gotarmor_type_legs_msg3], "%s", "SHOURUYKEN (UP+DASH)");
        sprintf(lines[strings_ingame_gotarmor_type_legs_msg4], "%s", " ");
        sprintf(lines[strings_ingame_gotarmor_type_body], "%s", "THIS FORTIFIED BODY WILL");
        sprintf(lines[strings_ingame_gotarmor_type_body_msg1], "%s", "BE INTANGIBLE MORE TIME");
        sprintf(lines[strings_ingame_gotarmor_type_body_msg2], "%s", "TAKE HALF DAMAGE");
        sprintf(lines[strings_ingame_gotarmor_type_body_msg3], "%s", "AVOID PUSH-BACK ON HIT");
        sprintf(lines[strings_ingame_gotarmor_type_body_msg4], "%s", "RESIST SPIKES");
        sprintf(lines[strings_ingame_difficulty_select], "%s", "SELECT DIFFICULTY:");
        sprintf(lines[strings_ingame_difficulty_easy], "%s", "EASY");
        sprintf(lines[strings_ingame_difficulty_normal], "%s", "NORMAL");
        sprintf(lines[strings_ingame_difficulty_hard], "%s", "HARD");
        sprintf(lines[strings_ingame_config_input_selected_joystick], "%s", "SELECTED JOYSTICK");
        sprintf(lines[strings_ingame_config_input_buttons], "%s", "CONFIG BUTTONS");
        sprintf(lines[strings_ingame_config_input_turbo_mode], "%s", "TURBO MODE");
        sprintf(lines[strings_ingame_config_on], "%s", "ON");
        sprintf(lines[strings_ingame_config_off], "%s", "OFF");
        sprintf(lines[strings_ingame_config_quitgame], "%s", "QUIT GAME");
        sprintf(lines[strings_ingame_armor], "%s", "ARMOR");
        sprintf(lines[strings_ingame_config_audio_volume_music], "%s", "MUSIC VOLUME");
        sprintf(lines[strings_ingame_config_audio_volume_sfx], "%s", "SFX VOLUME");

        sprintf(lines[strings_ingame_config_enabled], "%s", "ENABLED");
        sprintf(lines[strings_ingame_config_disabled], "%s", "DISABLED");

        sprintf(lines[strings_ingame_ready_message], "%s", "READY");


        /// @TODO: add assert to check that we set all the values from the enum

        std::vector<std::string> res;
        for (int i=0; i<strings_ingame_COUNT; i++) {
            std::string line = std::string(lines[i]) + std::string("\n");
            std::cout << "fio_strings::create_default_ingame_strings[" << i << "]: " << line << std::endl;
            res.push_back(line);
        }
        return res;
    }

    void fio_strings::create_default_ingame_strings()
    {

        std::vector<std::string> res = get_default_ingame_strings_list();
        save_game_strings(res, get_game_strings_filename());
        
    }

    std::vector<std::string> fio_strings::add_missing_default_ingame_strings(std::vector<std::string> list)
    {
        std::vector<std::string> res = get_default_ingame_strings_list();
        // add \n to the list that were removed when loaded from file
        for (int i=0; i<list.size(); i++) {
            list.at(i) = list.at(i) + std::string("\n");
        }
        for (int i=list.size(); i<strings_ingame_COUNT; i++) {
            std::cout << "ADD MISSING LINE: '" << res.at(i) << "'" << std::endl;
            list.push_back(res.at(i));
        }
        save_game_strings(list, get_game_strings_filename());
        return list;
    }

    std::string fio_strings::get_stage_dialogs_filename(short stage_id)
    {
        char char_filename[100];
        sprintf(char_filename, "/dialogs/stage_dialogs_%d.dat", stage_id);
        std::string filename = FILEPATH + std::string(char_filename);
        return filename;
    }

    void fio_strings::create_default_common_strings()
    {

        /// @TODO - set the IDS for each line

    }

    void fio_strings::create_default_dialog_strings()
    {
        std::vector<std::string> res;
        for (int i=0; i<FS_MAX_STAGES; i++) {
            char line_value[FS_CHAR_NAME_SIZE];
            // person dialogs
            for (int j=0; j<6; j++) {
                sprintf(line_value, "START-STG[%d] DLG #%d\n", i, j);
                res.push_back(line_value);
            }
            // players dialogs
            for (int j=0; j<4; j++) { // players
                for (int k=0; k<6; k++) { // phrases
                    char player_line_value[FS_COMMON_STRINGS_DIALOG];
                    sprintf(player_line_value, "START-STG[%d] DLG P[%d] %d\n", i, (j+1), k);
                    res.push_back(player_line_value);
                }
            }
            // person dialogs
            for (int j=0; j<6; j++) {
                sprintf(line_value, "BOSS STG[%d] DLG #%d\n", i, j);
                res.push_back(line_value);
            }
            // players dialogs
            for (int j=0; j<4; j++) { // players
                for (int k=0; k<6; k++) { // lines
                    char player_line_value[FS_COMMON_STRINGS_DIALOG];
                    sprintf(player_line_value, "BOSS STG[%d] DLG P[%d] %d\n", i, (j+1), k);
                    res.push_back(player_line_value);
                }
            }
            save_game_strings(res, get_stage_dialogs_filename(i));
            res.clear();
        }
    }




    std::vector<std::string> fio_strings::get_common_strings()
    {
        if (FILEPATH == "") {
            std::cout << "FIO_STRINGS - NO FILEPATH count: " << common_strings_list.size() << std::endl;
            return common_strings_list;
        }

        if (common_strings_list.size() == 0) {
            std::cout << "FIO_STRINGS - LOAD count: " << common_strings_list.size() << std::endl;
            common_strings_list = load_game_strings_from_file(get_common_strings_filename());
        }
        return common_strings_list;
    }

    std::string fio_strings::get_common_string(int id)
    {

        if (id == -1) {
            return std::string("");
        }
        if (FILEPATH == "") {
            return std::string("");
        }

        std::cout << "### fio_strings::get_common_string - id: " << id << std::endl;

        if (common_strings_list.size() == 0) {
            common_strings_list = load_game_strings_from_file(get_common_strings_filename());
        }

        if (id >= common_strings_list.size()) {
            return std::string("");
        }

        return common_strings_list.at(id);
    }

    std::string fio_strings::get_stage_dialog(short stage_id, int id)
    {
        if (_dialogs_stage_id != stage_id) {
            _dialogs_stage_id = stage_id;
            dialogs_strings_list = load_game_strings_from_file(get_stage_dialogs_filename(_dialogs_stage_id));
            if (dialogs_strings_list.size() == 0) {
                create_default_dialog_strings();
                dialogs_strings_list = load_game_strings_from_file(get_stage_dialogs_filename(_dialogs_stage_id));
            }
        }
        if (id < 0 || id >= dialogs_strings_list.size()) {
            return std::string("");
        }
        return dialogs_strings_list.at(id);
    }

    std::vector<std::string> fio_strings::get_stage_dialogs(short stage_id)
    {
        if (_dialogs_stage_id != stage_id) {
            _dialogs_stage_id = stage_id;
            dialogs_strings_list = load_game_strings_from_file(get_stage_dialogs_filename(_dialogs_stage_id));
            if (dialogs_strings_list.size() == 0) {
                create_default_dialog_strings();
                dialogs_strings_list = load_game_strings_from_file(get_stage_dialogs_filename(_dialogs_stage_id));
            }
        }
        // check size
        if (dialogs_strings_list.size() < 60) {
            std::cout << "Invalid dialogs list size[" << dialogs_strings_list.size() << "]. Minimum is 60." << std::endl;
            exit(-1);
        }
        return dialogs_strings_list;
    }


    void fio_strings::save_common_strings(std::vector<std::string> data)
    {
        save_game_strings(data, get_common_strings_filename());
    }

    void fio_strings::create_files()
    {
        create_default_ingame_strings();
        create_default_common_strings();
    }

    void fio_strings::save_stage_dialogs(short stage_id, std::vector<std::string> data)
    {
        save_game_strings(data, get_stage_dialogs_filename(stage_id));
    }










}// namepsace
