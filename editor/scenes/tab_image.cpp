#include "tab_image.h"
#include "ui_tab_image.h"
#include "common.h"

tab_image::tab_image(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tab_image)
{
    ui->setupUi(this);

    mediator = ScenesMediator::get_instance();

    fill_data();

    QObject::connect(ui->image_preview_widget, SIGNAL(on_image_w_changed(int)), this, SLOT(change_w(int)));
    QObject::connect(ui->image_preview_widget, SIGNAL(on_image_h_changed(int)), this, SLOT(change_h(int)));
}

tab_image::~tab_image()
{
    delete ui;
}

void tab_image::save_data()
{
    for (int i=0; i<mediator->image_list.size(); i++) {
        std::cout << "DEBUG #A, value: '" << mediator->image_list.at(i).name << "'" << std::endl;
    }
    fio.save_scenes_show_image(mediator->image_list);
}

void tab_image::change_fields_enabled(bool state)
{
    ui->select_comboBox->setEnabled(state);
    ui->name_textEdit->setEnabled(state);
    ui->filename_comboBox->setEnabled(state);
    ui->init_x_spinBox->setEnabled(state);
    ui->init_y_spinBox->setEnabled(state);
    ui->img_area_x->setEnabled(state);
    ui->img_area_y->setEnabled(state);
    ui->img_area_w->setEnabled(state);
    ui->img_area_h->setEnabled(state);
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

    mediator->image_list = fio.load_scenes_show_image();
    int list_size = mediator->image_list.size();
    std::cout << "list_size: " << list_size << std::endl;


    if (list_size == 0) {
        std::cout << "DEBUG #1" << std::endl;
        change_fields_enabled(false);
    } else {
        std::cout << "DEBUG #2" << std::endl;
        // fill
        for (int i=0; i<list_size; i++) {
            std::cout << "DEBUG #3, value: '" << mediator->image_list.at(i).name << "'" << std::endl;
            ui->select_comboBox->addItem(QString(mediator->image_list.at(i).name));
        }
    }
    data_loading = false;
    ui->select_comboBox->setCurrentIndex(0);
}

void tab_image::on_add_Button_clicked()
{
    CURRENT_FILE_FORMAT::file_scene_show_image new_image;
    sprintf(new_image.name, "%s%d", "Show Image #", mediator->image_list.size()+1);
    mediator->image_list.push_back(new_image);
    ui->select_comboBox->addItem(QString(new_image.name));
    if (mediator->image_list.size() == 1) {
        change_fields_enabled(true);
    }
    ui->select_comboBox->setCurrentIndex(mediator->image_list.size()-1);
}

void tab_image::on_select_comboBox_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ui->name_textEdit->setText(QString(mediator->image_list.at(index).name));
    ui->filename_comboBox->setCurrentIndex(ui->filename_comboBox->findText(QString(mediator->image_list.at(index).filename)));
    ui->init_x_spinBox->setValue(mediator->image_list.at(index).ini_x);
    ui->init_y_spinBox->setValue(mediator->image_list.at(index).ini_y);
    ui->destx_spinBox->setValue(mediator->image_list.at(index).dest_x);
    ui->desty_spinBox->setValue(mediator->image_list.at(index).dest_y);
    ui->delay_spinBox->setValue(mediator->image_list.at(index).move_delay);
    ui->img_area_x->setValue(mediator->image_list.at(index).copy_area.x);
    ui->img_area_y->setValue(mediator->image_list.at(index).copy_area.y);
    ui->img_area_w->setValue(mediator->image_list.at(index).copy_area.w);
    ui->img_area_h->setValue(mediator->image_list.at(index).copy_area.h);

    ui->blocking_checkBox->setChecked(mediator->image_list.at(index).blocking);
}

void tab_image::on_destx_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    mediator->image_list.at(ui->select_comboBox->currentIndex()).dest_x = arg1;
}

void tab_image::on_desty_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    mediator->image_list.at(ui->select_comboBox->currentIndex()).dest_y = arg1;
}

void tab_image::on_delay_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    mediator->image_list.at(ui->select_comboBox->currentIndex()).move_delay = arg1;
}

void tab_image::on_blocking_checkBox_toggled(bool checked)
{
    if (data_loading) { return; }
    mediator->image_list.at(ui->select_comboBox->currentIndex()).blocking = checked;
}

void tab_image::on_filename_comboBox_currentIndexChanged(const QString &arg1)
{
    if (data_loading) { return; }
    sprintf(mediator->image_list.at(ui->select_comboBox->currentIndex()).filename, "%s", arg1.toStdString().c_str());
    ui->image_preview_widget->setImageFilename(arg1);
    ui->image_preview_widget->repaint();
}

void tab_image::on_init_y_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    mediator->image_list.at(ui->select_comboBox->currentIndex()).ini_x = arg1;
}

void tab_image::on_init_x_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    mediator->image_list.at(ui->select_comboBox->currentIndex()).ini_y = arg1;
}

void tab_image::on_img_area_x_valueChanged(int arg1)
{
    if (data_loading) { return; }
    mediator->image_list.at(ui->select_comboBox->currentIndex()).copy_area.x = arg1;
    ui->image_preview_widget->set_x(arg1);
    ui->image_preview_widget->repaint();
}

void tab_image::on_img_area_y_valueChanged(int arg1)
{
    if (data_loading) { return; }
    mediator->image_list.at(ui->select_comboBox->currentIndex()).copy_area.y = arg1;
    ui->image_preview_widget->set_y(arg1);
    ui->image_preview_widget->repaint();
}

void tab_image::on_img_area_w_valueChanged(int arg1)
{
    if (data_loading) { return; }
    mediator->image_list.at(ui->select_comboBox->currentIndex()).copy_area.w = arg1;
    ui->image_preview_widget->set_w(arg1);
    ui->image_preview_widget->repaint();
}

void tab_image::on_img_area_h_valueChanged(int arg1)
{
    if (data_loading) { return; }
    mediator->image_list.at(ui->select_comboBox->currentIndex()).copy_area.h = arg1;
    ui->image_preview_widget->set_h(arg1);
    ui->image_preview_widget->repaint();
}

void tab_image::change_w(int value)
{
    data_loading = true;
    ui->img_area_w->setValue(value);
    data_loading = false;
}

void tab_image::change_h(int value)
{
    data_loading = true;
    ui->img_area_h->setValue(value);
    data_loading = false;
}
