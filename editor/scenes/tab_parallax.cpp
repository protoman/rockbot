#include "tab_parallax.h"
#include "ui_tab_parallax.h"
#include "common.h"

extern std::string GAMEPATH;
extern std::string GAMENAME;

tab_parallax::tab_parallax(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tab_parallax)
{
    ui->setupUi(this);

    fill_data();
}

tab_parallax::~tab_parallax()
{
    delete ui;
}

void tab_parallax::save_data()
{
    fio.save_scenes_parallax(ScenesMediator::get_instance()->parallax_list);
}

void tab_parallax::reload()
{
    fill_data();
}

void tab_parallax::fill_data()
{
    data_loading = true;
    ScenesMediator::get_instance()->parallax_list = fio.load_scenes_parallax();

    common::fill_files_combo("/images/scenes", ui->parallax_layer_image_comboBox);
    common::fill_direction_combo(ui->parallax_direction_comboBox);
    common::fill_numbered_combo(ui->parallax_layer_comboBox, 0, PARALLAX_LAYERS_MAX);
    int list_size = ScenesMediator::get_instance()->parallax_list.size();

    for (int i=0; i<list_size; i++) {
        ui->parallax_select_comboBox->addItem(QString(ScenesMediator::get_instance()->parallax_list.at(i).name));
    }
    data_loading = false;
    ui->parallax_select_comboBox->setCurrentIndex(0);

    set_fields(0);

    data_loading = false;

}

void tab_parallax::set_fields(int index)
{
    if (index < 0 || index >= ScenesMediator::get_instance()->parallax_list.size()) {
        return;
    }
    ui->parallax_name_lineEdit->setText(QString(ScenesMediator::get_instance()->parallax_list.at(index).name));
    ui->parallax_delay_spinBox->setValue(ScenesMediator::get_instance()->parallax_list.at(index).frame_delay);
    ui->parallax_direction_comboBox->setCurrentIndex(ScenesMediator::get_instance()->parallax_list.at(index).move_direction);
    ui->duration_spinBox->setValue(ScenesMediator::get_instance()->parallax_list.at(index).total_duration);
    ui->parallax_layer_comboBox->setCurrentIndex(0);
    set_layer_fields(0);
    set_all_layer_images(index);

}

void tab_parallax::set_layer_fields(int layer_n)
{
    int index = ui->parallax_select_comboBox->currentIndex();
    int count = ui->parallax_layer_comboBox->count();
    if (layer_n < 0 || layer_n >= count) {
        return;
    }

    ui->parallax_layer_image_comboBox->setCurrentIndex(ui->parallax_layer_image_comboBox->findText(ScenesMediator::get_instance()->parallax_list.at(index).filename[layer_n]));
    ui->parallax_layer_ypos_spinBox->setValue(ScenesMediator::get_instance()->parallax_list.at(index).adjust_y[layer_n]);
    ui->parallax_layer_maxh_spinBox->setValue(ScenesMediator::get_instance()->parallax_list.at(index).adjust_h[layer_n]);
    ui->parallax_layer_speed_spinBox->setValue(ScenesMediator::get_instance()->parallax_list.at(index).layer_speed[layer_n]);

    //update_preview_image(ui->select_comboBox->currentIndex());

}

void tab_parallax::set_all_layer_images(int parallax_n)
{
    for (int i=0; i<PARALLAX_LAYERS_MAX; i++) {
        std::string layer_filename = std::string(ScenesMediator::get_instance()->parallax_list.at(parallax_n).filename[i]);
        if (layer_filename.length() == 0) {
            ui->image_preview_widget->set_filename(i, "");
        } else {
            std::string filename = FILEPATH + "/images/scenes/" + layer_filename;
            ui->image_preview_widget->set_filename(i, filename);
        }
        ui->image_preview_widget->set_y(i, ScenesMediator::get_instance()->parallax_list.at(parallax_n).adjust_y[i]);
        ui->image_preview_widget->set_h(i, ScenesMediator::get_instance()->parallax_list.at(parallax_n).adjust_h[i]);
    }
}

void tab_parallax::update_preview_image(int parallax_n, int layer_n)
{
    std::string filename = FILEPATH + "/images/scenes/" + ScenesMediator::get_instance()->parallax_list.at(parallax_n).filename[layer_n];
    ui->image_preview_widget->set_filename(layer_n, filename);
    //ui->image_preview_widget->set_y(ScenesMediator::get_instance()->parallax_list.at(parallax_n).adjust_y[layer_n]);
    //ui->image_preview_widget->set_h(ScenesMediator::get_instance()->parallax_list.at(parallax_n).adjust_h[layer_n]);
    ui->image_preview_widget->repaint();
}

void tab_parallax::on_parallax_select_comboBox_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    data_loading = true;
    set_fields(index);
    data_loading = false;
}

void tab_parallax::on_pushButton_clicked()
{
    CURRENT_FILE_FORMAT::file_scene_show_parallax new_parallax;
    sprintf(new_parallax.name, "%s%d", "Parallax #", ScenesMediator::get_instance()->parallax_list.size()+1);
    ScenesMediator::get_instance()->parallax_list.push_back(new_parallax);
    ui->parallax_select_comboBox->addItem(QString(new_parallax.name));
    ui->parallax_select_comboBox->setCurrentIndex(ScenesMediator::get_instance()->parallax_list.size()-1);

}

void tab_parallax::on_parallax_layer_comboBox_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    data_loading = true;
    set_layer_fields(index);
    data_loading = false;
}

void tab_parallax::on_parallax_layer_image_comboBox_currentIndexChanged(const QString &arg1)
{
    if (data_loading) { return; }
    int current_parallax = ui->parallax_select_comboBox->currentIndex();
    int current_layer = ui->parallax_layer_comboBox->currentIndex();
    sprintf(ScenesMediator::get_instance()->parallax_list.at(current_parallax).filename[current_layer], "%s", arg1.toStdString().c_str());
    update_preview_image(current_parallax, current_layer);
    ui->image_preview_widget->repaint();
}

void tab_parallax::on_parallax_layer_ypos_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    int current_parallax = ui->parallax_select_comboBox->currentIndex();
    int current_layer = ui->parallax_layer_comboBox->currentIndex();
    ScenesMediator::get_instance()->parallax_list.at(current_parallax).adjust_y[current_layer] = arg1;
    ui->image_preview_widget->set_y(current_layer, arg1);
    ui->image_preview_widget->repaint();

}

void tab_parallax::on_parallax_layer_maxh_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    int current_parallax = ui->parallax_select_comboBox->currentIndex();
    int current_layer = ui->parallax_layer_comboBox->currentIndex();
    ScenesMediator::get_instance()->parallax_list.at(current_parallax).adjust_h[current_layer] = arg1;
    ui->image_preview_widget->set_h(current_layer, arg1);
    ui->image_preview_widget->repaint();

}

void tab_parallax::on_parallax_layer_speed_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    int current_parallax = ui->parallax_select_comboBox->currentIndex();
    int current_layer = ui->parallax_layer_comboBox->currentIndex();
    ScenesMediator::get_instance()->parallax_list.at(current_parallax).layer_speed[current_layer] = arg1;
}

void tab_parallax::on_run_pushButton_clicked()
{
    QString file = QString("scenesviewer");
#ifdef WIN32
    file += QString(".exe");
#endif
    file += QString(" --gamename \"") + QString(GAMENAME.c_str()) + QString("\"") + QString(" --parallax ") + QString::number(ui->parallax_select_comboBox->currentIndex());
    process.start(file);
}

void tab_parallax::on_parallax_delay_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    int current_parallax = ui->parallax_select_comboBox->currentIndex();
    ScenesMediator::get_instance()->parallax_list.at(current_parallax).frame_delay = arg1;
}

void tab_parallax::on_duration_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    int current_parallax = ui->parallax_select_comboBox->currentIndex();
    ScenesMediator::get_instance()->parallax_list.at(current_parallax).total_duration = arg1;
}
