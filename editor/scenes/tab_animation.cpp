#include "tab_animation.h"
#include "ui_tab_animation.h"

#include "mediator.h"
#include "common.h"

TabAnimation::TabAnimation(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabAnimation)
{
    ui->setupUi(this);

    fill_data();

    QObject::connect(ui->widget, SIGNAL(on_image_w_changed(int)), this, SLOT(change_w(int)));
    QObject::connect(ui->widget, SIGNAL(on_image_h_changed(int)), this, SLOT(change_h(int)));
}

TabAnimation::~TabAnimation()
{
    delete ui;
}

void TabAnimation::save_data()
{
    fio.save_scenes_show_animation(ScenesMediator::get_instance()->animation_list);
}

void TabAnimation::reload()
{
    fill_data();
}

void TabAnimation::change_fields_enabled(bool state)
{
    ui->name_lineEdit->setEnabled(state);
    ui->width_spinBox->setEnabled(state);
    ui->height_spinBox->setEnabled(state);
    ui->filename_comboBox->setEnabled(state);
    ui->duration_spinBox->setEnabled(state);
}

void TabAnimation::fill_data()
{
    data_loading = true;

    ScenesMediator::get_instance()->animation_list = fio.load_scenes_show_animation();

    common::fill_files_combo("/images/scenes/animations", ui->filename_comboBox);

    int list_size = ScenesMediator::get_instance()->animation_list.size();

    if (list_size == 0) {
        change_fields_enabled(false);
    } else {
        change_fields_enabled(true);
        for (int i=0; i<list_size; i++) {
            ui->select_comboBox->addItem(QString(ScenesMediator::get_instance()->animation_list.at(i).name));
        }
        set_fields(0);
    }
    data_loading = false;
    ui->select_comboBox->setCurrentIndex(0);
}

void TabAnimation::set_fields(int index)
{
    ui->name_lineEdit->setText(ScenesMediator::get_instance()->animation_list.at(index).name);
    ui->width_spinBox->setValue(ScenesMediator::get_instance()->animation_list.at(index).frame_w);
    ui->height_spinBox->setValue(ScenesMediator::get_instance()->animation_list.at(index).frame_h);
    ui->duration_spinBox->setValue(ScenesMediator::get_instance()->animation_list.at(index).frame_delay);
    ui->filename_comboBox->setCurrentIndex(ui->filename_comboBox->findText(ScenesMediator::get_instance()->animation_list.at(index).filename));

    ui->widget->set_filename(QString(ScenesMediator::get_instance()->animation_list.at(index).filename));
    ui->widget->set_w(ScenesMediator::get_instance()->animation_list.at(index).frame_w);
    ui->widget->set_h(ScenesMediator::get_instance()->animation_list.at(index).frame_h);
    ui->widget->set_delay(ScenesMediator::get_instance()->animation_list.at(index).frame_delay);

    ui->posx_spinBox->setValue(ScenesMediator::get_instance()->animation_list.at(index).x);
    ui->posy_spinBox->setValue(ScenesMediator::get_instance()->animation_list.at(index).y);
}

void TabAnimation::change_w(int value)
{
    data_loading = true;
    ui->width_spinBox->setValue(value);
    ScenesMediator::get_instance()->animation_list.at(ui->select_comboBox->currentIndex()).frame_w = value;
    data_loading = false;
}

void TabAnimation::change_h(int value)
{
    data_loading = true;
    ui->height_spinBox->setValue(value);
    ScenesMediator::get_instance()->animation_list.at(ui->select_comboBox->currentIndex()).frame_h = value;
    data_loading = false;
}

void TabAnimation::on_add_pushButton_clicked()
{
    CURRENT_FILE_FORMAT::file_scene_show_animation new_animation;
    sprintf(new_animation.name, "%s%d", "Show Animation #", ScenesMediator::get_instance()->animation_list.size()+1);
    ScenesMediator::get_instance()->animation_list.push_back(new_animation);
    ui->select_comboBox->addItem(QString(new_animation.name));
    if (ScenesMediator::get_instance()->animation_list.size() == 1) {
        change_fields_enabled(true);
    }
    ui->select_comboBox->setCurrentIndex(ScenesMediator::get_instance()->animation_list.size()-1);
}

void TabAnimation::on_select_comboBox_currentIndexChanged(int index)
{
    if (data_loading == true) { return; }
    set_fields(index);
    ScenesMediator::get_instance()->selected_scene = index;
}

void TabAnimation::on_name_lineEdit_textChanged(const QString &arg1)
{
    if (data_loading == true) { return; }
    sprintf(ScenesMediator::get_instance()->animation_list.at(ui->select_comboBox->currentIndex()).name, "%s", arg1.toStdString().c_str());
}

void TabAnimation::on_filename_comboBox_currentIndexChanged(const QString &arg1)
{
    if (data_loading == true) { return; }
    sprintf(ScenesMediator::get_instance()->animation_list.at(ui->select_comboBox->currentIndex()).filename, "%s", arg1.toStdString().c_str());
    ui->widget->set_filename(QString(ScenesMediator::get_instance()->animation_list.at(ui->select_comboBox->currentIndex()).filename));
    st_size img_size = common::calc_image_size(ScenesMediator::get_instance()->animation_list.at(ui->select_comboBox->currentIndex()).filename);
    ui->width_spinBox->setValue(img_size.width);
    ui->height_spinBox->setValue(img_size.height);
}

void TabAnimation::on_width_spinBox_valueChanged(int arg1)
{
    if (data_loading == true) { return; }
    ScenesMediator::get_instance()->animation_list.at(ui->select_comboBox->currentIndex()).frame_w = arg1;
    ui->widget->set_w(ScenesMediator::get_instance()->animation_list.at(ui->select_comboBox->currentIndex()).frame_w);
}

void TabAnimation::on_height_spinBox_valueChanged(int arg1)
{
    if (data_loading == true) { return; }
    ScenesMediator::get_instance()->animation_list.at(ui->select_comboBox->currentIndex()).frame_h = arg1;
    ui->widget->set_h(ScenesMediator::get_instance()->animation_list.at(ui->select_comboBox->currentIndex()).frame_h);
}

void TabAnimation::on_duration_spinBox_valueChanged(int arg1)
{
    if (data_loading == true) { return; }
    ScenesMediator::get_instance()->animation_list.at(ui->select_comboBox->currentIndex()).frame_delay = arg1;
    ui->widget->set_delay(ScenesMediator::get_instance()->animation_list.at(ui->select_comboBox->currentIndex()).frame_delay);
}

void TabAnimation::on_posx_spinBox_valueChanged(int arg1)
{
    if (data_loading == true) { return; }
    ScenesMediator::get_instance()->animation_list.at(ui->select_comboBox->currentIndex()).x = arg1;
    ui->widget->set_x(arg1);
}

void TabAnimation::on_posy_spinBox_valueChanged(int arg1)
{
    if (data_loading == true) { return; }
    ScenesMediator::get_instance()->animation_list.at(ui->select_comboBox->currentIndex()).y = arg1;
    ui->widget->set_y(arg1);
}

void TabAnimation::on_resetSize_pushButton_clicked()
{
    st_size img_size = common::calc_image_size(ScenesMediator::get_instance()->animation_list.at(ui->select_comboBox->currentIndex()).filename);
    ui->width_spinBox->setValue(img_size.width);
    ui->height_spinBox->setValue(img_size.height);
}
