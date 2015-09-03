#include "colorcycle.h"
#include "ui_colorcycle.h"
#include "dialog_pick_color.h"
#include "file/format.h"

#include "mediator.h"
extern Mediator *dataExchanger;
extern CURRENT_FILE_FORMAT::file_stages stage_data;

#include "common.h"

colorcycle_edit::colorcycle_edit(QWidget *parent) : QWidget(parent), ui(new Ui::colorcycle_edit), _selected_n(0), _selected_color_key(0), _data_loading(true)
{
	ui->setupUi(this);


	/// @TODO - show map in preview
}

colorcycle_edit::~colorcycle_edit()
{
    delete ui;
}

void colorcycle_edit::reload()
{
    _data_loading = true;
    _button_list.clear();
    _button_list.push_back(ui->color_pick_button1);
    _button_list.push_back(ui->color_pick_button2);
    _button_list.push_back(ui->color_pick_button3);
    _button_list.push_back(ui->color_pick_button4);
    _button_list.push_back(ui->color_pick_button5);
    _button_list.push_back(ui->color_pick_button6);
    _button_list.push_back(ui->color_pick_button7);
    _button_list.push_back(ui->color_pick_button8);
    _button_list.push_back(ui->color_pick_button9);
    _button_list.push_back(ui->color_pick_button10);

    _timer_list.clear();
    _timer_list.push_back(ui->duration1);
    _timer_list.push_back(ui->duration2);
    _timer_list.push_back(ui->duration3);
    _timer_list.push_back(ui->duration4);
    _timer_list.push_back(ui->duration5);
    _timer_list.push_back(ui->duration6);
    _timer_list.push_back(ui->duration7);
    _timer_list.push_back(ui->duration8);
    _timer_list.push_back(ui->duration9);
    _timer_list.push_back(ui->duration10);

    ui->stage_selector->clear();
    common::fill_stages_combo(ui->stage_selector);
    _data_loading = false;
    reload_colors();
}

void colorcycle_edit::pick_color(short n)
{
	_selected_n = n;
	QDialog *color_pick = new dialog_pick_color;
	color_pick->show();
	QObject::connect(color_pick, SIGNAL(accepted()), this, SLOT(pick_color_slot()));
}

void colorcycle_edit::reload_colors()
{
	if (_data_loading) {
		return;
	}
	// repaint buttons bg color
	std::stringstream ss;
	for (int i=0; i<10; i++) {
		ss.str(std::string());
		ss << "background-color: rgb(" << stage_data.stages[dataExchanger->currentStage].colorcycle.color[i][_selected_color_key].r << ", " << stage_data.stages[dataExchanger->currentStage].colorcycle.color[i][_selected_color_key].g << ", " << stage_data.stages[dataExchanger->currentStage].colorcycle.color[i][_selected_color_key].b << ")";
		_button_list[i]->setStyleSheet(ss.str().c_str());
	}
	// fill duration spinboxes
	for (int i=0; i<10; i++) {
        if (stage_data.stages[dataExchanger->currentStage].colorcycle.duration[i][_selected_color_key] <= 0) {
            stage_data.stages[dataExchanger->currentStage].colorcycle.duration[i][_selected_color_key] = 100;
        }
		_timer_list[i]->setValue(stage_data.stages[dataExchanger->currentStage].colorcycle.duration[i][_selected_color_key]);
	}
}

void colorcycle_edit::pick_color_slot()
{
	std::cout << "colorcycle_edit::pick_color_slot - _selected_color_key: " << _selected_color_key << std::endl;
	if (dataExchanger->picked_color_n != -1) {
		stage_data.stages[dataExchanger->currentStage].colorcycle.color[_selected_n][_selected_color_key].r = dataExchanger->colormap[dataExchanger->picked_color_n].r;
		stage_data.stages[dataExchanger->currentStage].colorcycle.color[_selected_n][_selected_color_key].g = dataExchanger->colormap[dataExchanger->picked_color_n].g;
		stage_data.stages[dataExchanger->currentStage].colorcycle.color[_selected_n][_selected_color_key].b = dataExchanger->colormap[dataExchanger->picked_color_n].b;
	} else {
		stage_data.stages[dataExchanger->currentStage].colorcycle.color[_selected_n][_selected_color_key].r = -1;
		stage_data.stages[dataExchanger->currentStage].colorcycle.color[_selected_n][_selected_color_key].g = -1;
		stage_data.stages[dataExchanger->currentStage].colorcycle.color[_selected_n][_selected_color_key].b = -1;
	}
	reload_colors();
}


void colorcycle_edit::on_stage_selector_currentIndexChanged(int index)
{
	dataExchanger->currentStage = index;
	reload_colors();
}

void colorcycle_edit::on_color_pick_button1_clicked()
{
	pick_color(0);
}

void colorcycle_edit::on_color_pick_button2_clicked()
{
	pick_color(1);
}

void colorcycle_edit::on_color_pick_button3_clicked()
{
	pick_color(2);
}

void colorcycle_edit::on_color_pick_button4_clicked()
{
	pick_color(3);
}

void colorcycle_edit::on_color_pick_button5_clicked()
{
	pick_color(4);
}

void colorcycle_edit::on_color_pick_button6_clicked()
{
	pick_color(5);
}

void colorcycle_edit::on_color_pick_button7_clicked()
{
	pick_color(6);
}

void colorcycle_edit::on_color_pick_button8_clicked()
{
	pick_color(7);
}

void colorcycle_edit::on_color_pick_button9_clicked()
{
	pick_color(8);
}

void colorcycle_edit::on_color_pick_button10_clicked()
{
	pick_color(9);
}

void colorcycle_edit::on_comboBox_2_currentIndexChanged(int index)
{
	dataExchanger->currentMap = index;
	ui->map_preview->repaint();
}

void colorcycle_edit::on_comboBox_currentIndexChanged(int index)
{
	_selected_color_key = index;
	reload_colors();
}

void colorcycle_edit::on_duration1_valueChanged(int arg1)
{
	stage_data.stages[dataExchanger->currentStage].colorcycle.duration[0][_selected_color_key] = arg1;
}

void colorcycle_edit::on_duration2_valueChanged(int arg1)
{
	stage_data.stages[dataExchanger->currentStage].colorcycle.duration[1][_selected_color_key] = arg1;
}

void colorcycle_edit::on_duration3_valueChanged(int arg1)
{
	stage_data.stages[dataExchanger->currentStage].colorcycle.duration[2][_selected_color_key] = arg1;
}

void colorcycle_edit::on_duration4_valueChanged(int arg1)
{
	stage_data.stages[dataExchanger->currentStage].colorcycle.duration[3][_selected_color_key] = arg1;
}

void colorcycle_edit::on_duration5_valueChanged(int arg1)
{
	stage_data.stages[dataExchanger->currentStage].colorcycle.duration[4][_selected_color_key] = arg1;
}

void colorcycle_edit::on_duration6_valueChanged(int arg1)
{
	stage_data.stages[dataExchanger->currentStage].colorcycle.duration[5][_selected_color_key] = arg1;
}

void colorcycle_edit::on_duration7_valueChanged(int arg1)
{
	stage_data.stages[dataExchanger->currentStage].colorcycle.duration[6][_selected_color_key] = arg1;
}

void colorcycle_edit::on_duration8_valueChanged(int arg1)
{
	stage_data.stages[dataExchanger->currentStage].colorcycle.duration[7][_selected_color_key] = arg1;
}

void colorcycle_edit::on_duration9_valueChanged(int arg1)
{
	stage_data.stages[dataExchanger->currentStage].colorcycle.duration[8][_selected_color_key] = arg1;
}

void colorcycle_edit::on_duration10_valueChanged(int arg1)
{
	stage_data.stages[dataExchanger->currentStage].colorcycle.duration[9][_selected_color_key] = arg1;
}

void colorcycle_edit::on_show_colorcycle1_checkbox_toggled(bool checked)
{
	dataExchanger->show_colorcycle1 = checked;
}

void colorcycle_edit::on_show_colorcycle2_checkbox_toggled(bool checked)
{
	dataExchanger->show_colorcycle2 = checked;
}

void colorcycle_edit::on_show_colorcycle3_checkbox_toggled(bool checked)
{
	dataExchanger->show_colorcycle3 = checked;
}
