#ifndef WEAPON_EDIT_H
#define WEAPON_EDIT_H

#include <QWidget>

#include "../file/format.h"

namespace Ui {
class weapon_edit;
}

class weapon_edit : public QWidget
{
	Q_OBJECT
	
public:
	explicit weapon_edit(QWidget *parent = 0);
	~weapon_edit();
    void reload();

private:
	void reload_weapon_list();
	
private slots:
	void on_weapon_select_combo_currentIndexChanged(int index);

	void on_weapon_name_textChanged(const QString &arg1);

	void on_weapon_projectile_type_currentIndexChanged(int index);

	void on_weapon_damage_valueChanged(int arg1);

    void on_weapon_charged_projectile_type_currentIndexChanged(int index);

private:
	Ui::weapon_edit *ui;
	short _selected_weapon;
	bool _loaded;
};

#endif // WEAPON_EDIT_H
