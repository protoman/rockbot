#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include "../../defines.h"
#include "../../file/format/st_hitPoints.h"
#include "../../file/v_3_0_0.h"
#include "../../file/v3/file_stage.h"

#include "../../file/v_3_0_1.h"
#include "../../file/v3/3_0_1/v301_stage.h"

std::string FILEPATH;
std::string SAVEPATH;

void read_all_stages_v300(format_v_3_0_0::file_stages& stages_data_out, std::string data_filename)
{
    std::ifstream fp;
    std::string filename = std::string(FILEPATH) + "data/" + data_filename;
    fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
    if (!fp.is_open()) {
        std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
        return;
    }
    fp.read(reinterpret_cast<char *>(&stages_data_out), sizeof(struct format_v_3_0_0::file_stages));
    fp.close();
}

void write_all_stages_v300(format_v_3_0_0::file_stages &stages_data_in)
{
    std::ofstream fp;
    std::string filename = std::string(FILEPATH) + "data/stages_v3.dat";
    fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
    if (!fp.is_open()) {
        std::cout << "ERROR::write_all_stages - could not write to file '" << filename << "'. Will create new one." << std::endl;
        fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
    } else {
        std::cout << "fio::write_game - recorded to file '" << filename << std::endl;
    }
    fp.write(reinterpret_cast<char *>(&stages_data_in), sizeof(struct format_v_3_0_0::file_stages));
    fp.close();
}


void read_all_stages_v301(format_v_3_0_1::file_stages& stages_data_out, std::string data_filename)
{
    std::ifstream fp;
    std::string filename = std::string(FILEPATH) + "data/" + data_filename;
    fp.open(filename.c_str(), std::ios::in | std::ios::binary | std::ios::app);
    if (!fp.is_open()) {
        std::cout << "ERROR::read_game - could not load file '" << filename << "'" << std::endl;
        return;
    }
    fp.read(reinterpret_cast<char *>(&stages_data_out), sizeof(struct format_v_3_0_1::file_stages));
    fp.close();
}

void write_all_stages_v301(format_v_3_0_1::file_stages &stages_data_in)
{
    std::ofstream fp;
    std::string filename = std::string(FILEPATH) + "data/stages_v301.dat";
    fp.open(filename.c_str(), std::ios::out | std::ios::binary | std::ios::ate);
    if (!fp.is_open()) {
        std::cout << "ERROR::write_all_stages - could not write to file '" << filename << "'. Will create new one." << std::endl;
        fp.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
    } else {
        std::cout << "fio::write_game - recorded to file '" << filename << std::endl;
    }
    fp.write(reinterpret_cast<char *>(&stages_data_in), sizeof(struct format_v_3_0_1::file_stages));
    fp.close();
}

int main(int argc, char *argv[])
{
	std::string EXEC_NAME("conversor");

#ifndef WIN32
	EXEC_NAME = "conversor";
#else
	EXEC_NAME = "conversor.exe";
#endif

	std::string argvString = std::string(argv[0]);
	FILEPATH = argvString.substr(0, argvString.size()-EXEC_NAME.size());

    format_v_3_0_1::file_stages stages_data;
    format_v_3_0_0::file_stages dialogs_data;
    std::cout << "Loading data...." << std::endl;
    read_all_stages_v301(stages_data, "stages_v301.dat");
    read_all_stages_v300(dialogs_data, "dialogs.dat");
    std::cout << "Copying data...." << std::endl;
    /*
    char line1[FS_DIALOG_LINES][DIALOG_LINE_LIMIT];
    char line2[FS_DIALOG_LINES][DIALOG_LINE_LIMIT];


    char answer1[FS_MAX_PLAYERS][FS_DIALOG_LINES][DIALOG_LINE_LIMIT];
    char answer2[FS_MAX_PLAYERS][FS_DIALOG_LINES][DIALOG_LINE_LIMIT];
    */



    for (int i=0; i<FS_MAX_STAGES; i++) {

        std::cout << "STAGE[" << i << "]" << std::endl;

        for (int k=0; k<FS_DIALOG_LINES; k++) {
            sprintf(stages_data.stages[i].intro_dialog.line1[k], "%s", dialogs_data.stages[i].intro_dialog.line1[k]);
            sprintf(stages_data.stages[i].intro_dialog.line2[k], "%s", dialogs_data.stages[i].intro_dialog.line2[k]);

            sprintf(stages_data.stages[i].boss_dialog.line1[k], "%s", dialogs_data.stages[i].boss_dialog.line1[k]);
            sprintf(stages_data.stages[i].boss_dialog.line2[k], "%s", dialogs_data.stages[i].boss_dialog.line2[k]);
        }

        for (int j=0; j<FS_MAX_PLAYERS; j++) {
            std::cout << "PLAYER[" << j << "]" << std::endl;
            for (int k=0; k<FS_DIALOG_LINES; k++) {
                std::cout << "answer1: " << stages_data.stages[i].intro_dialog.answer1[j][k] << std::endl;
                sprintf(stages_data.stages[i].intro_dialog.answer1[j][k], "%s", dialogs_data.stages[i].intro_dialog.answer1[j][k]);
                sprintf(stages_data.stages[i].intro_dialog.answer2[j][k], "%s", dialogs_data.stages[i].intro_dialog.answer2[j][k]);

                sprintf(stages_data.stages[i].boss_dialog.answer1[j][k], "%s", dialogs_data.stages[i].boss_dialog.answer1[j][k]);
                sprintf(stages_data.stages[i].boss_dialog.answer2[j][k], "%s", dialogs_data.stages[i].boss_dialog.answer2[j][k]);


            }
        }



    }
    std::cout << "Writing data...." << std::endl;
    write_all_stages_v301(stages_data);
    std::cout << "Done!" << std::endl;
}
