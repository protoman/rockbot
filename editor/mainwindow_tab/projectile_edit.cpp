#include "projectile_edit.h"
#include "ui_projectile_edit.h"

#include "common.h"

projectile_edit::projectile_edit(QWidget *parent) : QWidget(parent), ui(new Ui::projectile_edit)
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
    if (FILEPATH.length() == 0) {
        return;
    }

    data_loading = true;
    common::fill_projectiles_combo(ui->projectileList_combo);
    common::fill_files_combo(std::string("/images/projectiles"), ui->graphic_filename);
    common::fill_files_combo(std::string("/sfx"), ui->sfxFilename_comboBox);
	common::fill_trajectories_combo(ui->trajectory);
    Mediator::get_instance()->current_projectile = 0;
    set_edit_data(0);
    data_loading = false;
}

void projectile_edit::set_edit_data(int index)
{
    if (index < 0 || index >= Mediator::get_instance()->projectile_list_v2.size()) {
        return;
    }
    data_loading = true;
	Mediator::get_instance()->current_projectile = index;
    ui->name->setText(QString(Mediator::get_instance()->projectile_list_v2.at(index).name));
    ui->graphic_filename->setCurrentIndex(ui->graphic_filename->findText(QString(Mediator::get_instance()->projectile_list_v2.at(index).graphic_filename)));
    ui->sfxFilename_comboBox->setCurrentIndex(ui->sfxFilename_comboBox->findText(QString(Mediator::get_instance()->projectile_list_v2.at(index).sfx_filename)));
    ui->trajectory->setCurrentIndex(Mediator::get_instance()->projectile_list_v2.at(index).trajectory);
    ui->img_w->setValue(Mediator::get_instance()->projectile_list_v2.at(index).size.width);
    ui->img_h->setValue(Mediator::get_instance()->projectile_list_v2.at(index).size.height);
    if (Mediator::get_instance()->projectile_list_v2.at(index).is_destructible == true) {
		ui->projectileDestructibleCheckBox->setChecked(true);
	} else {
		ui->projectileDestructibleCheckBox->setChecked(false);
	}
    ui->projectileHitPointsSpinBox->setValue(Mediator::get_instance()->projectile_list_v2.at(index).hp);
    ui->max_shots->setValue(Mediator::get_instance()->projectile_list_v2.at(index).max_shots);
    ui->speed->setValue(Mediator::get_instance()->projectile_list_v2.at(index).speed);
    ui->damage->setValue(Mediator::get_instance()->projectile_list_v2.at(index).damage);

    if (Mediator::get_instance()->projectile_list_v2.size() == 0) {
        Mediator::get_instance()->projectile_list_v2.push_back(CURRENT_FILE_FORMAT::file_projectilev2());
    }

    ui->explosive_checkBox->setChecked(Mediator::get_instance()->projectile_list_v2.at(index).is_explosive);
	ui->projectilePreviewAreaWidget->repaint();
    data_loading = false;
}

void projectile_edit::on_projectileList_combo_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    Mediator::get_instance()->current_projectile = index;
    set_edit_data(index);
}

void projectile_edit::on_name_textChanged(const QString &arg1)
{
    if (data_loading) { return; }
    sprintf(Mediator::get_instance()->projectile_list_v2.at(Mediator::get_instance()->current_projectile).name, "%s", arg1.toStdString().c_str());
    ui->projectileList_combo->setItemText(Mediator::get_instance()->current_projectile, arg1);
}

void projectile_edit::on_graphic_filename_currentIndexChanged(const QString &arg1)
{
    if (data_loading) { return; }
    sprintf(Mediator::get_instance()->projectile_list_v2.at(Mediator::get_instance()->current_projectile).graphic_filename, "%s", arg1.toStdString().c_str());
	ui->projectilePreviewAreaWidget->repaint();
}


void projectile_edit::on_sfxFilename_comboBox_currentIndexChanged(const QString &arg1)
{
    if (data_loading) { return; }
    sprintf(Mediator::get_instance()->projectile_list_v2.at(Mediator::get_instance()->current_projectile).sfx_filename, "%s", arg1.toStdString().c_str());
}

void projectile_edit::on_trajectory_currentIndexChanged(int index)
{
    if (data_loading) { return; }
	PROJECTILE_TRAJECTORIES temp = (PROJECTILE_TRAJECTORIES)index;
    Mediator::get_instance()->projectile_list_v2.at(Mediator::get_instance()->current_projectile).trajectory = temp;
}

void projectile_edit::on_img_w_valueChanged(int arg1)
{
    if (data_loading) { return; }
    Mediator::get_instance()->projectile_list_v2.at(Mediator::get_instance()->current_projectile).size.width = arg1;
	ui->projectilePreviewAreaWidget->repaint();
}

void projectile_edit::on_img_h_valueChanged(int arg1)
{
    if (data_loading) { return; }
    Mediator::get_instance()->projectile_list_v2.at(Mediator::get_instance()->current_projectile).size.height = arg1;
	ui->projectilePreviewAreaWidget->repaint();
}

void projectile_edit::on_projectileDestructibleCheckBox_toggled(bool checked)
{
    if (data_loading) { return; }
    Mediator::get_instance()->projectile_list_v2.at(Mediator::get_instance()->current_projectile).is_destructible = checked;
}

void projectile_edit::on_projectileHitPointsSpinBox_valueChanged(int arg1)
{
    if (data_loading) { return; }
    Mediator::get_instance()->projectile_list_v2.at(Mediator::get_instance()->current_projectile).hp = arg1;
}

void projectile_edit::on_max_shots_valueChanged(int arg1)
{
    if (data_loading) { return; }
    Mediator::get_instance()->projectile_list_v2.at(Mediator::get_instance()->current_projectile).max_shots = arg1;
}

void projectile_edit::on_speed_valueChanged(int arg1)
{
    if (data_loading) { return; }
    Mediator::get_instance()->projectile_list_v2.at(Mediator::get_instance()->current_projectile).speed = arg1;
}

void projectile_edit::on_damage_valueChanged(int arg1)
{
    if (data_loading) { return; }
    Mediator::get_instance()->projectile_list_v2.at(Mediator::get_instance()->current_projectile).damage = arg1;
}

void projectile_edit::on_pushButton_clicked()
{
    Mediator::get_instance()->projectile_list_v2.push_back(CURRENT_FILE_FORMAT::file_projectilev2());
    ui->projectileList_combo->addItem(QString("[") + QString::number(Mediator::get_instance()->projectile_list_v2.size()-1) + QString("] Projectile Name"));
    ui->projectileList_combo->setCurrentIndex(Mediator::get_instance()->projectile_list_v2.size()-1);
}


void projectile_edit::on_explosive_checkBox_toggled(bool checked)
{
    if (data_loading) { return; }
    Mediator::get_instance()->projectile_list_v2.at(Mediator::get_instance()->current_projectile).is_explosive = checked;
}
