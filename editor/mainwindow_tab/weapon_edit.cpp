#include "weapon_edit.h"
#include "ui_weapon_edit.h"

#include "common.h"
#include "mediator.h"

weapon_edit::weapon_edit(QWidget *parent) :
QWidget(parent),
ui(new Ui::weapon_edit), _selected_weapon(0), _loaded(false)
{
	ui->setupUi(this);
	reload_weapon_list();
    common::fill_weapons_combo(ui->weapon_select_combo);
    common::fill_projectiles_combo(ui->weapon_projectile_type);
	_loaded = true;
}

weapon_edit::~weapon_edit()
{
    delete ui;
}

void weapon_edit::reload()
{
    _loaded = false;
    _selected_weapon = 0;
    reload_weapon_list();
    common::fill_weapons_combo(ui->weapon_select_combo);
    common::fill_projectiles_combo(ui->weapon_projectile_type);
    _loaded = true;
}

void weapon_edit::reload_weapon_list()
{
    common::fill_weapons_combo(ui->weapon_select_combo);
}

void weapon_edit::on_weapon_select_combo_currentIndexChanged(int index)
{
    _loaded = false;
    _selected_weapon = index;
    ui->weapon_name->setText(QString(Mediator::get_instance()->game_data.weapons[_selected_weapon].name));
    ui->weapon_damage->setValue(Mediator::get_instance()->game_data.weapons[_selected_weapon].damage);
    ui->weapon_projectile_type->setCurrentIndex(Mediator::get_instance()->game_data.weapons[_selected_weapon].id_projectile); // +1 because of the -1 default projectile
    ui->weapon_charged_projectile_type->setCurrentIndex(Mediator::get_instance()->game_data.weapons[_selected_weapon].id_projectile_charged); // +1 because of the -1 default projectile
    _loaded = true;
}

void weapon_edit::on_weapon_name_textChanged(const QString &arg1)
{
    if (_loaded == false) {
        return;
    }
    sprintf(Mediator::get_instance()->game_data.weapons[_selected_weapon].name, "%s", arg1.toStdString().c_str());
}

void weapon_edit::on_weapon_projectile_type_currentIndexChanged(int index)
{
    if (_loaded == false) {
        return;
    }
    Mediator::get_instance()->game_data.weapons[_selected_weapon].id_projectile = index;
}

void weapon_edit::on_weapon_damage_valueChanged(int arg1)
{
    if (_loaded == false) {
        return;
    }
    Mediator::get_instance()->game_data.weapons[_selected_weapon].damage = arg1;
}


void weapon_edit::on_weapon_charged_projectile_type_currentIndexChanged(int index)
{
    if (_loaded == false) { return; }
    Mediator::get_instance()->game_data.weapons[_selected_weapon].id_projectile_charged = index; //-1 is because default weapon (-1)
}
