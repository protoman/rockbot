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
    CURRENT_FILE_FORMAT::file_map temp_map[FS_STAGE_MAX_MAPS];
    for (int i=0; i<FS_STAGE_MAX_MAPS; i++) {
        temp_map[i] = Mediator::get_instance()->maps_data[dest_n][i];
    }
    for (int i=0; i<FS_STAGE_MAX_MAPS; i++) {
         Mediator::get_instance()->maps_data[dest_n][i] =  Mediator::get_instance()->maps_data[origin_n][i];
         Mediator::get_instance()->maps_data[origin_n][i] = temp_map[i];
    }

    // ### SWAP DIALOGS ### //

    // load data, if needed
    CURRENT_FILE_FORMAT::fio_strings fio_str;
    if (Mediator::get_instance()->stage_dialog_list.find(dest_n) == Mediator::get_instance()->stage_dialog_list.end()) {
        Mediator::get_instance()->stage_dialog_list.insert(std::pair<int, std::vector<std::string> >(dest_n, fio_str.get_stage_dialogs(dest_n, LANGUAGE_ENGLISH)));
    }
    if (Mediator::get_instance()->stage_dialog_list.find(origin_n) == Mediator::get_instance()->stage_dialog_list.end()) {
        Mediator::get_instance()->stage_dialog_list.insert(std::pair<int, std::vector<std::string> >(origin_n, fio_str.get_stage_dialogs(origin_n, LANGUAGE_ENGLISH)));
    }


    //std::map<int, std::vector<std::string> > stage_dialog_list;
    std::vector<std::string> temp_dialogs;
    // ==> dest to temp
    int dest_size = Mediator::get_instance()->stage_dialog_list.at(dest_n).size();
    for (int i=0; i<dest_size; i++) {
        temp_dialogs.push_back(Mediator::get_instance()->stage_dialog_list.at(dest_n)[i]);
    }

    // origin to dest
    Mediator::get_instance()->stage_dialog_list.at(dest_n).clear();
    for (int i=0; i<Mediator::get_instance()->stage_dialog_list.at(origin_n).size(); i++) {
        Mediator::get_instance()->stage_dialog_list.at(dest_n).push_back(Mediator::get_instance()->stage_dialog_list.at(ui->origin_combo->currentIndex())[i]);
    }

    // temp to origin
    Mediator::get_instance()->stage_dialog_list.at(origin_n).clear();
    for (int i=0; i<temp_dialogs.size(); i++) {
        Mediator::get_instance()->stage_dialog_list.at(origin_n).push_back(temp_dialogs[i]);
    }

    // ### SWAP FACES ### //
    char temp_face_name[FS_CHAR8_NAME_SIZE];
    sprintf(temp_face_name, "%s", Mediator::get_instance()->game_data.stages_face_name[dest_n]);
    sprintf(Mediator::get_instance()->game_data.stages_face_name[dest_n], "%s", Mediator::get_instance()->game_data.stages_face_name[origin_n]);
    sprintf(Mediator::get_instance()->game_data.stages_face_name[origin_n], "%s", temp_face_name);

    char temp_face_filename[FS_CHAR8_NAME_SIZE];
    sprintf(temp_face_filename, "%s", Mediator::get_instance()->game_data.stage_face_filename[dest_n]);
    sprintf(Mediator::get_instance()->game_data.stage_face_filename[dest_n], "%s", Mediator::get_instance()->game_data.stage_face_filename[origin_n]);
    sprintf(Mediator::get_instance()->game_data.stage_face_filename[origin_n], "%s", temp_face_filename);


}
