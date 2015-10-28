#include "tab_sfx.h"
#include "ui_tab_sfx.h"
#include "common.h"

TabSfx::TabSfx(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabSfx)
{
    ui->setupUi(this);

    fill_data();
}

TabSfx::~TabSfx()
{
    delete ui;
}

void TabSfx::change_fields_enabled(bool state)
{
    ui->filename_comboBox->setEnabled(state);
    ui->name_lineEdit->setEnabled(state);
}

void TabSfx::fill_data()
{
    data_loading = true;

    common::fill_files_combo("/sfx/", ui->filename_comboBox);
    ScenesMediator::get_instance()->playsfx_list = fio.load_scenes_play_sfx();
    int list_size = ScenesMediator::get_instance()->playsfx_list.size();

    if (list_size == 0) {
        change_fields_enabled(false);
    } else {
        change_fields_enabled(true);
        for (int i=0; i<list_size; i++) {
            ui->select_comboBox->addItem(QString(ScenesMediator::get_instance()->playsfx_list.at(i).name));
        }
        set_fields(0);

    }
    data_loading = false;
    ui->select_comboBox->setCurrentIndex(0);
}

void TabSfx::set_fields(int index)
{
    ui->filename_comboBox->setCurrentIndex(ui->filename_comboBox->findText(QString(ScenesMediator::get_instance()->playsfx_list.at(index).filename)));
    ui->name_lineEdit->setText(ScenesMediator::get_instance()->playsfx_list.at(index).name);
}

void TabSfx::save_data()
{
    fio.save_scenes_play_sfx(ScenesMediator::get_instance()->playsfx_list);
}

void TabSfx::reload()
{
    fill_data();
}

void TabSfx::on_add_pushButton_clicked()
{
    CURRENT_FILE_FORMAT::file_scene_play_sfx new_sfx;
    sprintf(new_sfx.name, "%s%d", "Play SFX #", ScenesMediator::get_instance()->playsfx_list.size()+1);
    ScenesMediator::get_instance()->playsfx_list.push_back(new_sfx);
    ui->select_comboBox->addItem(QString(new_sfx.name));
    if (ScenesMediator::get_instance()->playsfx_list.size() == 1) {
        change_fields_enabled(true);
    }
    ui->select_comboBox->setCurrentIndex(ScenesMediator::get_instance()->playsfx_list.size()-1);

}

void TabSfx::on_select_comboBox_currentIndexChanged(int index)
{
    if (data_loading == true) { return; }
    set_fields(index);
}

void TabSfx::on_name_lineEdit_textChanged(const QString &arg1)
{
    if (data_loading == true) { return; }
    sprintf(ScenesMediator::get_instance()->playsfx_list.at(ui->select_comboBox->currentIndex()).name, "%s", arg1.toStdString().c_str());
}

void TabSfx::on_filename_comboBox_currentIndexChanged(const QString &arg1)
{
    if (data_loading == true) { return; }
    sprintf(ScenesMediator::get_instance()->playsfx_list.at(ui->select_comboBox->currentIndex()).filename, "%s", arg1.toStdString().c_str());

}
