#include "object_tab.h"
#include "ui_object_tab.h"
#include "common.h"

object_tab::object_tab(QWidget *parent) :
    QWidget(parent),
	ui(new Ui::object_tab),
	_current_object(0),
	_data_loaded(false)
{
    ui->setupUi(this);

}

object_tab::~object_tab()
{
    delete ui;
}

void object_tab::reload()
{
    _current_object = 0;
    fill_data();
}

void object_tab::fill_data()
{
    _data_loaded = false;
    common::fill_files_combo("/images/sprites/objects", ui->graphicfile_combo);
    ui->object_preview_area->set_graphicfile(FILEPATH+std::string("/images/sprites/objects/")+std::string(Mediator::get_instance()->object_list.at(0).graphic_filename));
    ui->object_preview_area->repaint();

    common::fill_object_combo(ui->objectlist_combo);
    _data_loaded = true;
    on_objectlist_combo_currentIndexChanged(0);
}

void object_tab::on_graphicfile_combo_currentIndexChanged(const QString &arg1)
{
	if (_data_loaded == false) {
		return;
	}
    sprintf(Mediator::get_instance()->object_list.at(_current_object).graphic_filename, "%s", arg1.toStdString().c_str());
    ui->object_preview_area->set_graphicfile(FILEPATH+std::string("/images/sprites/objects/")+arg1.toStdString());
    ui->object_preview_area->repaint();
}

void object_tab::on_objectlist_combo_currentIndexChanged(int index)
{
	if (_data_loaded == false) {
		return;
	}

	_current_object = index;
    ui->name->setText(Mediator::get_instance()->object_list.at(index).name);
    ui->graphicfile_combo->setCurrentIndex(ui->graphicfile_combo->findText(QString(Mediator::get_instance()->object_list.at(index).graphic_filename)));
    ui->graphic_w->setValue(Mediator::get_instance()->object_list.at(index).size.width);
    ui->graphic_h->setValue(Mediator::get_instance()->object_list.at(index).size.height);
    ui->type_combo->setCurrentIndex(Mediator::get_instance()->object_list.at(index).type);
    ui->timer->setValue(Mediator::get_instance()->object_list.at(index).timer);
    ui->speed->setValue(Mediator::get_instance()->object_list.at(index).speed);
    ui->limit->setValue(Mediator::get_instance()->object_list.at(index).limit);

    ui->animate_on_active_checkbox->setChecked(!Mediator::get_instance()->object_list.at(_current_object).animation_auto_start);
    ui->animation_loop_checkbox->setChecked(Mediator::get_instance()->object_list.at(_current_object).animation_loop);
    ui->frame_duration_spinbox->setValue(Mediator::get_instance()->object_list.at(index).frame_duration);
    ui->animation_reverse_checkbox->setChecked(Mediator::get_instance()->object_list.at(_current_object).animation_reverse);
}

void object_tab::on_name_textChanged(const QString &arg1)
{
    if (_data_loaded == false) { return; }
    sprintf(Mediator::get_instance()->object_list.at(_current_object).name, "%s", arg1.toStdString().c_str());
    ui->objectlist_combo->setItemText(_current_object, QString(Mediator::get_instance()->object_list.at(_current_object).name));
}

void object_tab::on_type_combo_currentIndexChanged(int index)
{
	if (_data_loaded == false) {
		return;
	}
    Mediator::get_instance()->object_list.at(_current_object).type = index;
}

void object_tab::on_limit_valueChanged(int arg1)
{
    if (_data_loaded == false) { return; }
    Mediator::get_instance()->object_list.at(_current_object).limit = arg1;
}

void object_tab::on_graphic_w_valueChanged(int arg1)
{
    if (_data_loaded == false) { return; }
	ui->object_preview_area->set_grid_w(arg1);
    Mediator::get_instance()->object_list.at(_current_object).size.width = arg1;
}

void object_tab::on_graphic_h_valueChanged(int arg1)
{
    if (_data_loaded == false) { return; }
	ui->object_preview_area->set_grid_h(arg1);
    Mediator::get_instance()->object_list.at(_current_object).size.height = arg1;
}

void object_tab::on_speed_valueChanged(int arg1)
{
    if (_data_loaded == false) { return; }
    Mediator::get_instance()->object_list.at(_current_object).speed = arg1;
}

void object_tab::on_animate_on_active_checkbox_toggled(bool checked)
{
    if (_data_loaded == false) { return; }
    Mediator::get_instance()->object_list.at(_current_object).animation_auto_start = !checked;
}

void object_tab::on_animation_loop_checkbox_toggled(bool checked)
{
    if (_data_loaded == false) { return; }
    Mediator::get_instance()->object_list.at(_current_object).animation_loop = checked;
}

void object_tab::on_frame_duration_spinbox_valueChanged(int arg1)
{
    if (_data_loaded == false) { return; }
    Mediator::get_instance()->object_list.at(_current_object).frame_duration = arg1;
}

void object_tab::on_animation_reverse_checkbox_toggled(bool checked)
{
    if (_data_loaded == false) { return; }
    Mediator::get_instance()->object_list.at(_current_object).animation_reverse = checked;
}

void object_tab::on_timer_valueChanged(int arg1)
{
    if (_data_loaded == false) { return; }
    Mediator::get_instance()->object_list.at(_current_object).timer = arg1;
}

void object_tab::on_pushButton_clicked()
{
    Mediator::get_instance()->object_list.push_back(CURRENT_FILE_FORMAT::file_object());
    ui->objectlist_combo->addItem(QString("[") + QString::number(Mediator::get_instance()->object_list.size()-1) + QString("] Object"));
    ui->objectlist_combo->setCurrentIndex(Mediator::get_instance()->object_list.size()-1);

}
