#include "tab_changecolorcycle.h"
#include "ui_tab_changecolorcycle.h"

#include "mediator.h"
#include "common.h"

TabChangeColorcycle::TabChangeColorcycle(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabChangeColorcycle)
{
    ui->setupUi(this);

    fill_data();
}

TabChangeColorcycle::~TabChangeColorcycle()
{
    delete ui;
}

void TabChangeColorcycle::save_data()
{
    fio.save_scenes_change_colorcyle(ScenesMediator::get_instance()->changecolorcycle_list);
}

void TabChangeColorcycle::change_fields_enabled(bool state)
{
    ui->name_lineEdit->setEnabled(state);
    ui->colorcycle_comboBox->setEnabled(state);
}

void TabChangeColorcycle::fill_data()
{
    data_loading = true;

    ScenesMediator::get_instance()->changecolorcycle_list = fio.load_scenes_change_colorcycle();
    int list_size = ScenesMediator::get_instance()->changecolorcycle_list.size();

    common::fill_stages_combo(ui->colorcycle_comboBox);

    if (list_size == 0) {
        change_fields_enabled(false);
    } else {
        // fill
        for (int i=0; i<list_size; i++) {
            ui->select_comboBox->addItem(QString(ScenesMediator::get_instance()->changecolorcycle_list.at(i).name));
        }

        set_fields(0);
    }
    data_loading = false;
    ui->select_comboBox->setCurrentIndex(0);
}

void TabChangeColorcycle::set_fields(int index)
{
    ui->name_lineEdit->setText(ScenesMediator::get_instance()->changecolorcycle_list.at(index).name);
    ui->colorcycle_comboBox->setCurrentIndex(ScenesMediator::get_instance()->changecolorcycle_list.at(index).colorcycle_n);
}

void TabChangeColorcycle::on_select_comboBox_currentIndexChanged(int index)
{
    if (data_loading == true) { return; }
    set_fields(index);
}

void TabChangeColorcycle::on_name_lineEdit_textChanged(const QString &arg1)
{
    if (data_loading == true) { return; }
    sprintf(ScenesMediator::get_instance()->changecolorcycle_list.at(ui->select_comboBox->currentIndex()).name, "%s", arg1.toStdString().c_str());
}

void TabChangeColorcycle::on_colorcycle_comboBox_currentIndexChanged(int index)
{
    if (data_loading == true) { return; }
    ScenesMediator::get_instance()->changecolorcycle_list.at(ui->select_comboBox->currentIndex()).colorcycle_n = index;
}

void TabChangeColorcycle::on_add_pushButton_clicked()
{
    CURRENT_FILE_FORMAT::file_scene_change_colorcyle new_changecolorcycle;
    sprintf(new_changecolorcycle.name, "%s%d", "Change Colorcycle #", ScenesMediator::get_instance()->changecolorcycle_list.size()+1);
    ScenesMediator::get_instance()->changecolorcycle_list.push_back(new_changecolorcycle);
    ui->select_comboBox->addItem(QString(new_changecolorcycle.name));
    if (ScenesMediator::get_instance()->changecolorcycle_list.size() == 1) {
        change_fields_enabled(true);
    }
    ui->select_comboBox->setCurrentIndex(ScenesMediator::get_instance()->changecolorcycle_list.size()-1);

}
