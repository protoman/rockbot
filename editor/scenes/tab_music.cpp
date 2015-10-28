#include "tab_music.h"
#include "ui_tab_music.h"
#include "common.h"

TabMusic::TabMusic(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabMusic)
{
    ui->setupUi(this);

    fill_data();
}

TabMusic::~TabMusic()
{
    delete ui;
}

void TabMusic::change_fields_enabled(bool state)
{
    ui->filename_comboBox->setEnabled(state);
    ui->name_lineEdit->setEnabled(state);
}

void TabMusic::fill_data()
{
    data_loading = true;

    common::fill_files_combo("/music/", ui->filename_comboBox);
    ScenesMediator::get_instance()->playmusic_list = fio.load_scenes_play_music();
    int list_size = ScenesMediator::get_instance()->playmusic_list.size();

    if (list_size == 0) {
        change_fields_enabled(false);
    } else {
        change_fields_enabled(true);
        for (int i=0; i<list_size; i++) {
            ui->select_comboBox->addItem(QString(ScenesMediator::get_instance()->playmusic_list.at(i).name));
        }
        set_fields(0);

    }
    data_loading = false;
    ui->select_comboBox->setCurrentIndex(0);
}

void TabMusic::set_fields(int index)
{
    ui->filename_comboBox->setCurrentIndex(ui->filename_comboBox->findText(QString(ScenesMediator::get_instance()->playmusic_list.at(index).filename)));
    ui->name_lineEdit->setText(ScenesMediator::get_instance()->playmusic_list.at(index).name);
}

void TabMusic::save_data()
{
    fio.save_scenes_play_music(ScenesMediator::get_instance()->playmusic_list);
}

void TabMusic::reload()
{
    fill_data();
}

void TabMusic::on_add_pushButton_clicked()
{
    CURRENT_FILE_FORMAT::file_scene_play_music new_music;
    sprintf(new_music.name, "%s%d", "Play Music #", ScenesMediator::get_instance()->playmusic_list.size()+1);
    ScenesMediator::get_instance()->playmusic_list.push_back(new_music);
    ui->select_comboBox->addItem(QString(new_music.name));
    if (ScenesMediator::get_instance()->playmusic_list.size() == 1) {
        change_fields_enabled(true);
    }
    ui->select_comboBox->setCurrentIndex(ScenesMediator::get_instance()->playmusic_list.size()-1);

}

void TabMusic::on_select_comboBox_currentIndexChanged(int index)
{
    if (data_loading == true) { return; }
    set_fields(index);
}

void TabMusic::on_name_lineEdit_textChanged(const QString &arg1)
{
    if (data_loading == true) { return; }
    sprintf(ScenesMediator::get_instance()->playmusic_list.at(ui->select_comboBox->currentIndex()).name, "%s", arg1.toStdString().c_str());
}

void TabMusic::on_filename_comboBox_currentIndexChanged(const QString &arg1)
{
    if (data_loading == true) { return; }
    sprintf(ScenesMediator::get_instance()->playmusic_list.at(ui->select_comboBox->currentIndex()).filename, "%s", arg1.toStdString().c_str());

}

