#include "stageselectmapdialog.h"
#include "ui_stageselectmapdialog.h"
#include "mediator.h"

StageSelectMapDialog::StageSelectMapDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StageSelectMapDialog)
{
    ui->setupUi(this);
}

StageSelectMapDialog::~StageSelectMapDialog()
{
    delete ui;
}

void StageSelectMapDialog::on_path_pushButton_clicked()
{
    ui->path_pushButton->setChecked(true);
    ui->stage_pushButton->setChecked(false);
    ui->castle_pushButton->setChecked(false);
    Mediator::get_instance()->stage_select_edit_mode = STAGE_SELECT_EDIT_MODE_PATH;

    std::cout << "### set.stage_select_edit_mode[" << (int)Mediator::get_instance()->stage_select_edit_mode << "]" << std::endl;
}

void StageSelectMapDialog::on_stage_pushButton_clicked()
{
    ui->path_pushButton->setChecked(false);
    ui->stage_pushButton->setChecked(true);
    ui->castle_pushButton->setChecked(false);
    Mediator::get_instance()->stage_select_edit_mode = STAGE_SELECT_EDIT_MODE_STAGE;

    std::cout << "### set.stage_select_edit_mode[" << (int)Mediator::get_instance()->stage_select_edit_mode << "]" << std::endl;
}

void StageSelectMapDialog::on_castle_pushButton_clicked()
{
    ui->path_pushButton->setChecked(false);
    ui->stage_pushButton->setChecked(false);
    ui->castle_pushButton->setChecked(true);
    Mediator::get_instance()->stage_select_edit_mode = STAGE_SELECT_EDIT_MODE_CASTLE;

    std::cout << "### set.stage_select_edit_mode[" << (int)Mediator::get_instance()->stage_select_edit_mode << "]" << std::endl;

}

void StageSelectMapDialog::on_stageNumber_spinBox_valueChanged(int arg1)
{
    Mediator::get_instance()->stage_select_stage_number = arg1;
}
