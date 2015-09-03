#include "projectile_edit.h"
#include "ui_projectile_edit.h"

#include "common.h"

projectile_edit::projectile_edit(QWidget *parent) : QWidget(parent), ui(new Ui::projectile_edit), _ignore_name_change(false)
{
    ui->setupUi(this);
    fill_data();
}

projectile_edit::~projectile_edit()
{
    delete ui;
}

void projectile_edit::reload()
{
    fill_data();
}

void projectile_edit::fill_data()
{
    common::fill_projectiles_combo(ui->projectileList_combo);
	common::fill_files_combo(std::string("data/images/projectiles"), ui->graphic_filename);
	common::fill_trajectories_combo(ui->trajectory);
    dataExchanger->current_projectile = -1;
    set_edit_data(-1);
}

// @TODO - set defaultedit_data

void projectile_edit::set_edit_data(int index)
{
	_ignore_name_change = true;
	dataExchanger->current_projectile = index;
	ui->name->setText(QString(game_data.projectiles[index].name));
	ui->graphic_filename->setCurrentIndex(ui->graphic_filename->findText(QString(game_data.projectiles[index].graphic_filename)));
    ui->trajectory->setCurrentIndex(game_data.projectiles[index].trajectory);
    ui->img_w->setValue(game_data.projectiles[index].size.width);
    ui->img_h->setValue(game_data.projectiles[index].size.height);
	if (game_data.projectiles[index].is_destructible == true) {
		ui->projectileDestructibleCheckBox->setChecked(true);
	} else {
		ui->projectileDestructibleCheckBox->setChecked(false);
	}
	ui->projectileHitPointsSpinBox->setValue(game_data.projectiles[index].hp);
	ui->max_shots->setValue(game_data.projectiles[index].max_shots);
	ui->speed->setValue(game_data.projectiles[index].speed);
	ui->damage->setValue(game_data.projectiles[index].damage);
	// if project is DEFAUL, disable all fields
	if (dataExchanger->current_projectile == -1) {
		ui->name->setDisabled(true);
		ui->graphic_filename->setDisabled(true);
		ui->trajectory->setDisabled(true);
		ui->img_w->setDisabled(true);
		ui->img_h->setDisabled(true);
		ui->projectileDestructibleCheckBox->setDisabled(true);
		ui->projectileHitPointsSpinBox->setDisabled(true);
		ui->max_shots->setDisabled(true);
		ui->speed->setDisabled(true);
		ui->damage->setDisabled(true);
	} else {
		ui->name->setDisabled(false);
		ui->graphic_filename->setDisabled(false);
		ui->trajectory->setDisabled(false);
		ui->img_w->setDisabled(false);
		ui->img_h->setDisabled(false);
		ui->projectileDestructibleCheckBox->setDisabled(false);
		ui->projectileHitPointsSpinBox->setDisabled(false);
		ui->max_shots->setDisabled(false);
		ui->speed->setDisabled(false);
		ui->damage->setDisabled(false);
	}
	ui->projectilePreviewAreaWidget->repaint();
}

void projectile_edit::on_projectileList_combo_currentIndexChanged(int index)
{
	dataExchanger->current_projectile = index-1;
	set_edit_data(index-1);
}

void projectile_edit::on_name_textChanged(const QString &arg1)
{
	if (_ignore_name_change == true) {
		_ignore_name_change = false;
	} else {
		sprintf(game_data.projectiles[dataExchanger->current_projectile].name, "%s", arg1.toStdString().c_str());
		ui->projectileList_combo->setItemText(dataExchanger->current_projectile, arg1);
	}
}

void projectile_edit::on_graphic_filename_currentIndexChanged(const QString &arg1)
{
	sprintf(game_data.projectiles[dataExchanger->current_projectile].graphic_filename, "%s", arg1.toStdString().c_str());
	ui->projectilePreviewAreaWidget->repaint();
}

void projectile_edit::on_trajectory_currentIndexChanged(int index)
{
	PROJECTILE_TRAJECTORIES temp = (PROJECTILE_TRAJECTORIES)index;
	game_data.projectiles[dataExchanger->current_projectile].trajectory = temp;
}

void projectile_edit::on_img_w_valueChanged(int arg1)
{
	game_data.projectiles[dataExchanger->current_projectile].size.width = arg1;
	ui->projectilePreviewAreaWidget->repaint();
}

void projectile_edit::on_img_h_valueChanged(int arg1)
{
	game_data.projectiles[dataExchanger->current_projectile].size.height = arg1;
	ui->projectilePreviewAreaWidget->repaint();
}

void projectile_edit::on_projectileDestructibleCheckBox_toggled(bool checked)
{
	game_data.projectiles[dataExchanger->current_projectile].is_destructible = checked;
}

void projectile_edit::on_projectileHitPointsSpinBox_valueChanged(int arg1)
{
	game_data.projectiles[dataExchanger->current_projectile].hp = arg1;
}

void projectile_edit::on_max_shots_valueChanged(int arg1)
{
	game_data.projectiles[dataExchanger->current_projectile].max_shots = arg1;
}

void projectile_edit::on_speed_valueChanged(int arg1)
{
	game_data.projectiles[dataExchanger->current_projectile].speed = arg1;
}

void projectile_edit::on_damage_valueChanged(int arg1)
{
	game_data.projectiles[dataExchanger->current_projectile].damage = arg1;
}
