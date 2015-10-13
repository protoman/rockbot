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
