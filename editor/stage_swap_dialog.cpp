#include <QMessageBox>

#include "stage_swap_dialog.h"
#include "ui_stage_swap_dialog.h"
#include "common.h"

#include <file/file_io.h>

extern CURRENT_FILE_FORMAT::file_stages stage_data;


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
    if (ui->origin_combo->currentIndex() == ui->destiny_combo->currentIndex()) {
        QMessageBox msgBox;
        msgBox.setText("You can't swap between the same positions!");
        msgBox.exec();
        return;
    }
    CURRENT_FILE_FORMAT::file_stage temp_stage;
    temp_stage = stage_data.stages[ui->destiny_combo->currentIndex()];
    stage_data.stages[ui->destiny_combo->currentIndex()] =  stage_data.stages[ui->origin_combo->currentIndex()];
    stage_data.stages[ui->origin_combo->currentIndex()] =  temp_stage;
}
