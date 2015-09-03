#include "tab_image.h"
#include "ui_tab_image.h"
#include "common.h"

tab_image::tab_image(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tab_image)
{
    ui->setupUi(this);

    fill_data();
}

tab_image::~tab_image()
{
    delete ui;
}

void tab_image::save_data()
{
    for (int i=0; i<list.size(); i++) {
        std::cout << "DEBUG #A, value: '" << list.at(i).name << "'" << std::endl;
    }
    fio.save_scenes_show_image(list);
}

void tab_image::change_fields_enabled(bool state)
{
    ui->select_comboBox->setEnabled(state);
    ui->name_textEdit->setEnabled(state);
    ui->filename_comboBox->setEnabled(state);
    ui->x_spinBox->setEnabled(state);
    ui->y_spinBox->setEnabled(state);
    ui->destx_spinBox->setEnabled(state);
    ui->desty_spinBox->setEnabled(state);
    ui->delay_spinBox->setEnabled(state);
}

void tab_image::fill_data()
{
    data_loading = true;

    std::cout << "DEBUG #0" << std::endl;
    common::fill_files_combo("data/images/scenes", ui->filename_comboBox);
    std::cout << "DEBUG #0.1" << std::endl;

    list = fio.load_scenes_show_image();
    int list_size = list.size();
    std::cout << "list_size: " << list_size << std::endl;


    if (list_size == 0) {
        std::cout << "DEBUG #1" << std::endl;
        change_fields_enabled(false);
    } else {
        std::cout << "DEBUG #2" << std::endl;
        // fill
        for (int i=0; i<list_size; i++) {
            std::cout << "DEBUG #3, value: '" << list.at(i).name << "'" << std::endl;
            ui->select_comboBox->addItem(QString(list.at(i).name));
        }
    }
    data_loading = false;
    ui->select_comboBox->setCurrentIndex(0);
}

void tab_image::on_add_Button_clicked()
{
    CURRENT_FILE_FORMAT::file_scene_show_image new_image;
    sprintf(new_image.name, "%s%d", "Show Image #", list.size()+1);
    list.push_back(new_image);
    ui->select_comboBox->addItem(QString(new_image.name));
    if (list.size() == 1) {
        change_fields_enabled(true);
    }
    ui->select_comboBox->setCurrentIndex(list.size()-1);
}

void tab_image::on_select_comboBox_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ui->name_textEdit->setText(QString(list.at(index).name));
    ui->filename_comboBox->setCurrentIndex(ui->filename_comboBox->findText(QString(list.at(index).filename)));
    ui->x_spinBox->setValue(list.at(index).x);
    ui->y_spinBox->setValue(list.at(index).y);
    ui->destx_spinBox->setValue(list.at(index).dest_x);
    ui->desty_spinBox->setValue(list.at(index).dest_y);
    ui->delay_spinBox->setValue(list.at(index).move_delay);
    ui->blocking_checkBox->setChecked(list.at(index).blocking);
}

void tab_image::on_x_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    list.at(ui->select_comboBox->currentIndex()).x = arg1;
}

void tab_image::on_y_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    list.at(ui->select_comboBox->currentIndex()).y = arg1;
}

void tab_image::on_destx_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    list.at(ui->select_comboBox->currentIndex()).dest_x = arg1;
}

void tab_image::on_desty_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    list.at(ui->select_comboBox->currentIndex()).dest_y = arg1;
}

void tab_image::on_delay_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    list.at(ui->select_comboBox->currentIndex()).move_delay = arg1;
}

void tab_image::on_blocking_checkBox_toggled(bool checked)
{
    if (data_loading) { return; }
    list.at(ui->select_comboBox->currentIndex()).blocking = checked;
}

void tab_image::on_filename_comboBox_currentIndexChanged(const QString &arg1)
{
    if (data_loading) { return; }
    sprintf(list.at(ui->select_comboBox->currentIndex()).filename, "%s", arg1.toStdString().c_str());
}
