#include "tab_viewpoint.h"
#include "ui_tab_viewpoint.h"
#include "common.h"


tab_viewpoint::tab_viewpoint(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tab_viewpoint)
{
    ui->setupUi(this);

    fill_data();

    QObject::connect(ui->image_preview_widget, SIGNAL(on_image_w_changed(int)), this, SLOT(change_w(int)));
    QObject::connect(ui->image_preview_widget, SIGNAL(on_image_h_changed(int)), this, SLOT(change_h(int)));

}

tab_viewpoint::~tab_viewpoint()
{
    delete ui;
}



void tab_viewpoint::save_data()
{
    fio.save_scenes_show_viewpoint(ScenesMediator::get_instance()->viewpoint_list);
}

void tab_viewpoint::reload()
{
    fill_data();
}


void tab_viewpoint::change_fields_enabled(bool state)
{
    ui->select_comboBox->setEnabled(state);
    ui->name_textEdit->setEnabled(state);
    ui->filename_comboBox->setEnabled(state);
    ui->init_x_spinBox->setEnabled(state);
    ui->init_y_spinBox->setEnabled(state);
    ui->img_area_w->setEnabled(state);
    ui->img_area_h->setEnabled(state);
    ui->destx_spinBox->setEnabled(state);
    ui->desty_spinBox->setEnabled(state);
    ui->delay_spinBox->setEnabled(state);
}



void tab_viewpoint::fill_data()
{
    data_loading = true;

    common::fill_files_combo("/images/scenes", ui->filename_comboBox);
    ScenesMediator::get_instance()->viewpoint_list = fio.load_scenes_show_viewpoint();
    int list_size = ScenesMediator::get_instance()->viewpoint_list.size();

    if (list_size == 0) {
        change_fields_enabled(false);
    } else {
        change_fields_enabled(true);
        for (int i=0; i<list_size; i++) {
            ui->select_comboBox->addItem(QString(ScenesMediator::get_instance()->viewpoint_list.at(i).name));
        }
        set_fields(0);

    }
    data_loading = false;
    ui->select_comboBox->setCurrentIndex(0);
}

void tab_viewpoint::change_w(int value)
{
    data_loading = true;
    ui->img_area_w->setValue(value);
    data_loading = false;

}

void tab_viewpoint::change_h(int value)
{
    data_loading = true;
    ui->img_area_h->setValue(value);
    data_loading = false;

}

void tab_viewpoint::set_fields(int index)
{
    ui->name_textEdit->setText(QString(ScenesMediator::get_instance()->viewpoint_list.at(index).name));
    ui->filename_comboBox->setCurrentIndex(ui->filename_comboBox->findText(QString(ScenesMediator::get_instance()->viewpoint_list.at(index).filename)));
    ui->init_x_spinBox->setValue(ScenesMediator::get_instance()->viewpoint_list.at(index).ini_x);
    ui->init_y_spinBox->setValue(ScenesMediator::get_instance()->viewpoint_list.at(index).ini_y);
    ui->destx_spinBox->setValue(ScenesMediator::get_instance()->viewpoint_list.at(index).dest_x);
    ui->desty_spinBox->setValue(ScenesMediator::get_instance()->viewpoint_list.at(index).dest_y);
    ui->delay_spinBox->setValue(ScenesMediator::get_instance()->viewpoint_list.at(index).move_delay);
    ui->img_area_w->setValue(ScenesMediator::get_instance()->viewpoint_list.at(index).w);
    ui->img_area_h->setValue(ScenesMediator::get_instance()->viewpoint_list.at(index).h);

    ui->blocking_checkBox->setChecked(ScenesMediator::get_instance()->viewpoint_list.at(index).blocking);

    ui->select_comboBox->setCurrentIndex(ScenesMediator::get_instance()->viewpoint_list.at(index).loop_mode);

    ui->posx_spinBox->setValue( ScenesMediator::get_instance()->viewpoint_list.at(index).pos_x);
    ui->posy_spinBox->setValue( ScenesMediator::get_instance()->viewpoint_list.at(index).pos_y);


    update_preview_image(0);

}

void tab_viewpoint::update_preview_image(int index)
{
    QString filename = QString(FILEPATH.c_str()) + QString("/images/scenes/") + QString(ScenesMediator::get_instance()->viewpoint_list.at(index).filename);
    ui->image_preview_widget->setImageFilename(filename);
    ui->image_preview_widget->repaint();
}



void tab_viewpoint::on_filename_comboBox_currentIndexChanged(const QString &arg1)
{
    if (data_loading) { return; }
    sprintf(ScenesMediator::get_instance()->viewpoint_list.at(ui->select_comboBox->currentIndex()).filename, "%s", arg1.toStdString().c_str());
    st_size img_size = common::calc_image_size(ScenesMediator::get_instance()->animation_list.at(ui->select_comboBox->currentIndex()).filename);
    ui->img_area_w->setValue(img_size.width);
    ui->img_area_h->setValue(img_size.height);
    update_preview_image(ui->select_comboBox->currentIndex());
}

void tab_viewpoint::on_add_Button_clicked()
{
    CURRENT_FILE_FORMAT::file_scene_show_viewpoint new_viewpoint;
    sprintf(new_viewpoint.name, "%s%d", "Show Viewpoint #", ScenesMediator::get_instance()->viewpoint_list.size()+1);
    ScenesMediator::get_instance()->viewpoint_list.push_back(new_viewpoint);
    ui->select_comboBox->addItem(QString(new_viewpoint.name));
    if (ScenesMediator::get_instance()->viewpoint_list.size() == 1) {
        change_fields_enabled(true);
    }
    ui->select_comboBox->setCurrentIndex(ScenesMediator::get_instance()->viewpoint_list.size()-1);
}

void tab_viewpoint::on_select_comboBox_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    set_fields(index);
}

void tab_viewpoint::on_name_textEdit_textChanged(const QString &arg1)
{
    if (data_loading) { return; }
    sprintf(ScenesMediator::get_instance()->viewpoint_list.at(ui->select_comboBox->currentIndex()).name, "%s", arg1.toStdString().c_str());
}

void tab_viewpoint::on_init_x_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->viewpoint_list.at(ui->select_comboBox->currentIndex()).ini_x = arg1;
}

void tab_viewpoint::on_init_y_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->viewpoint_list.at(ui->select_comboBox->currentIndex()).ini_y = arg1;
}

void tab_viewpoint::on_destx_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->viewpoint_list.at(ui->select_comboBox->currentIndex()).dest_x = arg1;
}

void tab_viewpoint::on_desty_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->viewpoint_list.at(ui->select_comboBox->currentIndex()).dest_y = arg1;
}

void tab_viewpoint::on_img_area_w_valueChanged(int arg1)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->viewpoint_list.at(ui->select_comboBox->currentIndex()).w = arg1;
    ui->image_preview_widget->set_w(arg1);
    ui->image_preview_widget->repaint();

}

void tab_viewpoint::on_img_area_h_valueChanged(int arg1)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->viewpoint_list.at(ui->select_comboBox->currentIndex()).h = arg1;
    ui->image_preview_widget->set_h(arg1);
    ui->image_preview_widget->repaint();

}

void tab_viewpoint::on_delay_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->viewpoint_list.at(ui->select_comboBox->currentIndex()).move_delay = arg1;
}

void tab_viewpoint::on_comboBox_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->viewpoint_list.at(ui->select_comboBox->currentIndex()).loop_mode = index;
}

void tab_viewpoint::on_posx_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->viewpoint_list.at(ui->select_comboBox->currentIndex()).pos_x = arg1;
}

void tab_viewpoint::on_posy_spinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->viewpoint_list.at(ui->select_comboBox->currentIndex()).pos_y = arg1;
}

void tab_viewpoint::on_pushButton_clicked()
{
    st_size img_size = common::calc_image_size(ScenesMediator::get_instance()->animation_list.at(ui->select_comboBox->currentIndex()).filename);
    ui->img_area_w->setValue(img_size.width);
    ui->img_area_h->setValue(img_size.height);
}
