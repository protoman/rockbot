#include <QMessageBox>

#include "stage_swap_dialog.h"
#include "ui_stage_swap_dialog.h"
#include "common.h"
#include "mediator.h"

#include <file/file_io.h>

stage_swap_dialog::stage_swap_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::stage_swap_dialog)
{
    ui->setupUi(this);
    common::fill_stages_combo(ui->origin_combo);
    common::fill_stages_combo(ui->destiny_combo);
}

stage_swap_dialog::~stage_swap_dialog()
{
    delete ui;
}

void stage_swap_dialog::on_buttonBox_accepted()
{
    int dest_n = ui->destiny_combo->currentIndex();
    int origin_n = ui->origin_combo->currentIndex();
    if (origin_n == dest_n) {
        QMessageBox msgBox;
        msgBox.setText("You can't swap between the same positions!");
        msgBox.exec();
        return;
    }
    // ### SWAP STAGES ### //
    CURRENT_FILE_FORMAT::file_stage temp_stage;
    temp_stage = Mediator::get_instance()->stage_data.stages[dest_n];
    Mediator::get_instance()->stage_data.stages[dest_n] =  Mediator::get_instance()->stage_data.stages[origin_n];
    Mediator::get_instance()->stage_data.stages[origin_n] =  temp_stage;

    // ### SWAP MAPS ### //
    CURRENT_FILE_FORMAT::file_map_v2 temp_map[FS_STAGE_MAX_MAPS];
    for (int i=0; i<FS_STAGE_MAX_MAPS; i++) {
        temp_map[i] = Mediator::get_instance()->maps_data_v2[dest_n][i];
    }
    for (int i=0; i<FS_STAGE_MAX_MAPS; i++) {
         Mediator::get_instance()->maps_data_v2[dest_n][i] =  Mediator::get_instance()->maps_data_v2[origin_n][i];
         Mediator::get_instance()->maps_data_v2[origin_n][i] = temp_map[i];
    }

    // ### SWAP DIALOGS ### //
    for (int i=0; i<LANGUAGE_COUNT; i++) { // load data, if needed
        CURRENT_FILE_FORMAT::fio_strings fio_str;
        if (Mediator::get_instance()->stage_dialog_list[i].find(dest_n) == Mediator::get_instance()->stage_dialog_list[i].end()) {
            Mediator::get_instance()->stage_dialog_list[i].insert(std::pair<int, std::vector<std::string> >(dest_n, fio_str.get_stage_dialogs(dest_n, LANGUAGE_ENGLISH, false)));
        }
        if (Mediator::get_instance()->stage_dialog_list[i].find(origin_n) == Mediator::get_instance()->stage_dialog_list[i].end()) {
            Mediator::get_instance()->stage_dialog_list[i].insert(std::pair<int, std::vector<std::string> >(origin_n, fio_str.get_stage_dialogs(origin_n, LANGUAGE_ENGLISH, false)));
        }
        //std::map<int, std::vector<std::string> > stage_dialog_list;
        std::vector<std::string> temp_dialogs;
        // ==> dest to temp
        int dest_size = Mediator::get_instance()->stage_dialog_list[i].at(dest_n).size();
        for (int j=0; j<dest_size; j++) {
            temp_dialogs.push_back(Mediator::get_instance()->stage_dialog_list[i].at(dest_n)[j]);
        }

        // origin to dest
        Mediator::get_instance()->stage_dialog_list[i].at(dest_n).clear();
        for (int j=0; j<Mediator::get_instance()->stage_dialog_list[i].at(origin_n).size(); j++) {
            Mediator::get_instance()->stage_dialog_list[i].at(dest_n).push_back(Mediator::get_instance()->stage_dialog_list[i].at(ui->origin_combo->currentIndex())[j]);
        }

        // temp to origin
        Mediator::get_instance()->stage_dialog_list[i].at(origin_n).clear();
        for (int j=0; j<temp_dialogs.size(); j++) {
            Mediator::get_instance()->stage_dialog_list[i].at(origin_n).push_back(temp_dialogs[j]);
        }
    }

    // ### SWAP FACES ### //
    char temp_face_name[FS_CHAR8_NAME_SIZE];
    sprintf(temp_face_name, "%s", Mediator::get_instance()->game_data.stages_face_name[dest_n]);
    sprintf(Mediator::get_instance()->game_data.stages_face_name[dest_n], "%s", Mediator::get_instance()->game_data.stages_face_name[origin_n]);
    sprintf(Mediator::get_instance()->game_data.stages_face_name[origin_n], "%s", temp_face_name);

    char temp_face_filename[FS_FACE_FILENAME_MAX];
    sprintf(temp_face_filename, "%s", Mediator::get_instance()->game_data.stage_face_filename[dest_n]);
    sprintf(Mediator::get_instance()->game_data.stage_face_filename[dest_n], "%s", Mediator::get_instance()->game_data.stage_face_filename[origin_n]);
    sprintf(Mediator::get_instance()->game_data.stage_face_filename[origin_n], "%s", temp_face_filename);


    // SWAP OBJECTS
    for (unsigned int i=0; i<Mediator::get_instance()->maps_data_object_list.size(); i++) {
        if (Mediator::get_instance()->maps_data_object_list.at(i).stage_id == origin_n) {
            Mediator::get_instance()->maps_data_object_list.at(i).stage_id = dest_n;
        } else if (Mediator::get_instance()->maps_data_object_list.at(i).stage_id == dest_n) {
            Mediator::get_instance()->maps_data_object_list.at(i).stage_id = origin_n;
        }
    }

    // SWAP ENEMIES
    for (unsigned int i=0; i<Mediator::get_instance()->maps_data_npc_list.size(); i++) {
        if (Mediator::get_instance()->maps_data_npc_list.at(i).stage_id == origin_n) {
            Mediator::get_instance()->maps_data_npc_list.at(i).stage_id = dest_n;
        } else if (Mediator::get_instance()->maps_data_npc_list.at(i).stage_id == dest_n) {
            Mediator::get_instance()->maps_data_npc_list.at(i).stage_id = origin_n;
        }
    }

    QString tileset(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].tileset_filename);
    if (tileset.length() > 0) {
        Mediator::get_instance()->setPallete(tileset.toStdString());
    } else {
        Mediator::get_instance()->setPallete("default.png");
    }


    emit finished_swap_stages();
}
