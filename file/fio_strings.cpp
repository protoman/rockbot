#include "fio_strings.h"

extern std::string FILEPATH;

fio_strings::fio_strings()
{
}


namespace format_v4 {

    fio_strings::fio_strings()
    {
        string_list = load_game_strings();
    }

    std::string fio_strings::get_ingame_string(int n)
    {
        if (n < 0 || n > string_list.size()) {
            return "";
        }
        return string_list.at(n);
    }


    std::vector<std::string> fio_strings::load_game_strings()
    {
        std::string filename = std::string(FILEPATH) + "/strings_ingame_v4.dat";
        std::vector<std::string> res;
        FILE *fp = fopen(filename.c_str(), "rb");

        if (!fp) {
            std::cout << "[WARNING] file_io::load_game_strings - file '" << filename << "' not found, will generate default..." << std::endl;
            create_default_ingame_strings();
            fp = fopen(filename.c_str(), "rb");
            if (!fp) {
                std::cout << "[WARNING] file_io::load_game_strings - Critical error, can't open' '" << filename << "' for reading." << std::endl;
            }
        }

        while (!feof(fp) && !ferror(fp)) {
            st_strings_line out;
            int res_read = fread(&out, sizeof(st_strings_line), 1, fp);
            if (res_read == -1) {
                std::cout << ">>file_io::load_from_disk - Error reading data from scenes_list file '" << filename << "'." << std::endl;
                exit(-1);
            } else if (res_read == 1) {
                res.push_back(std::string(out.line));
            }
        }
        std::cout << ">>file_io::load_from_disk - size: " << res.size() << std::endl;
        fclose(fp);
        return res;
    }

    void fio_strings::save_game_strings(std::vector<std::string> list)
    {
        std::string filename = std::string(FILEPATH) + "/strings_ingame_v4.dat";
        FILE *fp = fopen(filename.c_str(), "wb");
        if (!fp) {
            std::cout << ">> fio_strings::save_game_strings: Could not open '" << filename << "' for writting." << std::endl;
            return;
        }

        int block_size = sizeof(st_strings_line);
        for (int i=0; i<list.size(); i++) {
            fwrite(list.at(i).c_str(), block_size, 1, fp);
        }
        fclose(fp);

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

    void fio_strings::create_default_ingame_strings()
    {
        struct st_strings_line lines[strings_ingame_COUNT];

        sprintf(lines[strings_ingame_newgame].line, "%s", "NEW GAME");
        sprintf(lines[strings_ingame_loadgame].line, "%s", "LOAD GAME");
        sprintf(lines[strings_ingame_config].line, "%s", "CONFIG");
        sprintf(lines[strings_ingame_about].line, "%s", "ABOUT");
        sprintf(lines[strings_ingame_password].line, "%s", "PASSWORD");
        sprintf(lines[strings_ingame_gameover].line, "%s", "GAME OVER");
        sprintf(lines[strings_ingame_and].line, "%s", "AND");
        sprintf(lines[strings_ingame_selectgame].line, "%s", "SELECT GAME");
        sprintf(lines[strings_ingame_savegameerror1].line, "%s", "ERROR WHILE SAVING GAME,");
        sprintf(lines[strings_ingame_savegameerror2].line, "%s", "PLEASE CHECK THAT THE DEVICE OR");
        sprintf(lines[strings_ingame_savegameerror3].line, "%s", "FILE IS NOT WRITE-PROTECTED.");
        sprintf(lines[strings_ingame_copyrightline].line, "%s", "\xA9 2009-2015 UPPERLAND STUDIOS");
        sprintf(lines[strings_ingame_audio].line, "%s", "AUDIO");
        sprintf(lines[strings_ingame_input].line, "%s", "INPUT");
        sprintf(lines[strings_ingame_video].line, "%s", "VIDEO");
        sprintf(lines[strings_ingame_leavestage].line, "%s", "LEAVE STAGE");
        sprintf(lines[strings_ingame_video_windowed].line, "%s", "WINDOWED");
        sprintf(lines[strings_ingame_video_fullscreen].line, "%s", "FULLSCREEN");
        sprintf(lines[strings_ingame_video_noscale].line, "%s", "NO SCALE  ");
        sprintf(lines[strings_ingame_video_size2x].line, "%s", "SIZE2X    ");
        sprintf(lines[strings_ingame_video_scale2x].line, "%s", "SCALE2X   ");
        sprintf(lines[strings_ingame_config_restart1].line, "%s", "PLEASE RESTART THE GAME");
        sprintf(lines[strings_ingame_config_restart2].line, "%s", "FOR THE CONFIGURATION");
        sprintf(lines[strings_ingame_config_restart3].line, "%s", "TO TAKE EFFECT");
        sprintf(lines[strings_ingame_config_presstorestart].line, "%s", "PRESS A BUTTON TO RETURN");
        sprintf(lines[strings_ingame_pressanykey].line, "%s", "PRESS ANY KEY OR BUTTON");
        sprintf(lines[strings_ingame_config_set].line, "%s", "SET");
        sprintf(lines[strings_ingame_config_key_up].line, "%s", "UP");
        sprintf(lines[strings_ingame_config_key_down].line, "%s", "DOWN");
        sprintf(lines[strings_ingame_config_key_left].line, "%s", "LEFT");
        sprintf(lines[strings_ingame_config_key_right].line, "%s", "RIGHT");
        sprintf(lines[strings_ingame_config_key_jump].line, "%s", "JUMP");
        sprintf(lines[strings_ingame_config_key_dash].line, "%s", "DASH");
        sprintf(lines[strings_ingame_config_key_fire].line, "%s", "FIRE");
        sprintf(lines[strings_ingame_config_key_start].line, "%s", "START");
        sprintf(lines[strings_ingame_config_key_keyl].line, "%s", "BUTTON-L");
        sprintf(lines[strings_ingame_config_key_keyr].line, "%s", "BUTTON-R");
        sprintf(lines[strings_ingame_config_key_pressnew].line, "%s", "PRESS NEW KEY/BUTTON");
        sprintf(lines[strings_ingame_config_key_reset].line, "%s", "RESET TO DEFAULT");
        sprintf(lines[strings_ingame_config_key_directional].line, "%s", "DIRECTIONAL");
        sprintf(lines[strings_ingame_config_key_directional_analog].line, "%s", "DIGITAL");
        sprintf(lines[strings_ingame_config_key_directional_digital].line, "%s", "ANALOG");

        sprintf(lines[strings_ingame_passwordinvalid].line, "%s", "PASSWORD INVALID");
        sprintf(lines[strings_ingame_pressstart].line, "%s", "PRESS START");

        sprintf(lines[strings_ingame_engineerror].line, "%s", "ROCKBOT ENGIONE ERROR");
        sprintf(lines[strings_ingame_nogames].line, "%s", "NO GAMES AVAILABLE");
        sprintf(lines[strings_ingame_quitgame].line, "%s", "QUIT GAME?");
        sprintf(lines[strings_ingame_life].line, "%s", "LIFE");
        sprintf(lines[strings_ingame_item].line, "%s", "ITEM");

        sprintf(lines[strings_ingame_yes].line, "%s", "YES");
        sprintf(lines[strings_ingame_no].line, "%s", "NO");


        sprintf(lines[strings_ingame_gotarmor_type_ability].line, "%s", "GIVE YOU THE ABILITY TO");
        sprintf(lines[strings_ingame_gotarmor_type_arms].line, "%s", "THIS IMPROVED ARMS WILL");
        sprintf(lines[strings_ingame_gotarmor_type_arms_msg1].line, "%s", "FIRE ALWAYS CHARGED");
        sprintf(lines[strings_ingame_gotarmor_type_arms_msg2].line, "%s", "CHARGE A LASER BEAM");
        sprintf(lines[strings_ingame_gotarmor_type_arms_msg3].line, "%s", "FIRE A SUPER-SHOT!");
        sprintf(lines[strings_ingame_gotarmor_type_arms_msg4].line, "%s", "THROW A HADOUKEN");
        sprintf(lines[strings_ingame_gotarmor_type_legs].line, "%s", "THOSE LIGHTER LEGS");
        sprintf(lines[strings_ingame_gotarmor_type_legs_msg1].line, "%s", "DASH IN MIDDLE-AIR.");
        sprintf(lines[strings_ingame_gotarmor_type_legs_msg2].line, "%s", "EXECUTE DOUBLE JUMP");
        sprintf(lines[strings_ingame_gotarmor_type_legs_msg3].line, "%s", "SHOURUYKEN (UP+DASH)");
        sprintf(lines[strings_ingame_gotarmor_type_legs_msg4].line, "%s", "");
        sprintf(lines[strings_ingame_gotarmor_type_body].line, "%s", "THIS FORTIFIED BODY WILL");
        sprintf(lines[strings_ingame_gotarmor_type_body_msg1].line, "%s", "BE INTANGIBLE MORE TIME");
        sprintf(lines[strings_ingame_gotarmor_type_body_msg2].line, "%s", "TAKE HALF DAMAGE");
        sprintf(lines[strings_ingame_gotarmor_type_body_msg3].line, "%s", "AVOID PUSH-BACK ON HIT");
        sprintf(lines[strings_ingame_gotarmor_type_body_msg4].line, "%s", "RESIST SPIKES");

        /// @TODO: add assert to check that we set all the values from the enum


        std::string filename = std::string(FILEPATH) + "/strings_ingame_v4.dat";
        FILE *fp = fopen(filename.c_str(), "wb");
        if (!fp) {
            std::cout << ">> fio_strings::create_default_ingame_strings: Could not open '" << filename << "' for writting." << std::endl;
            return;
        }

        int block_size = sizeof(st_strings_line);
        for (int i=0; i<strings_ingame_COUNT; i++) {
            fwrite(&lines[i], block_size, 1, fp);
        }
        fclose(fp);

    }


}// namepsace
