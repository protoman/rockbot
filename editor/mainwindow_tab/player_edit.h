#ifndef PLAYER_EDIT_H
#define PLAYER_EDIT_H

#include <QWidget>

#include "dialog_pick_color.h"
#include "mediator.h"

namespace Ui {
class player_edit;
}

class player_edit : public QWidget
{
    Q_OBJECT
    
public:
    explicit player_edit(QWidget *parent = 0);
    ~player_edit();
    void reload();


public slots:
    void pick_bg_color();
    void on_color_selected1(const QColor & color);
    void on_color_selected2(const QColor & color);
    void on_color_selected3(const QColor & color);
    void pick_player_color1();
    void pick_player_color2();
    void pick_player_color3();

private slots:
    void on_players_tab_list_combo_currentIndexChanged(int index);

    void on_players_tab_name_textChanged(const QString &arg1);

    void on_players_tab_hp_valueChanged(int arg1);

    void on_players_tab_hasshield_toggled(bool checked);

    void on_players_tab_maxshots_valueChanged(int arg1);

    void on_can_slide_checkbox_toggled(bool checked);

    void on_players_tab_list_combo_2_currentIndexChanged(int index);

    void on_player_graphics_combo_currentIndexChanged(const QString &arg1);

    void on_player_sprite_w_valueChanged(int arg1);

    void on_player_sprite_h_valueChanged(int arg1);

    void on_player_hitarea_x_valueChanged(int arg1);

    void on_player_hitarea_y_valueChanged(int arg1);

    void on_player_hitarea_w_valueChanged(int arg1);

    void on_player_hitarea_h_valueChanged(int arg1);

    void on_color1_picker_clicked();

    void on_color2_picker_clicked();

    void on_color3_picker_clicked();

    void on_chargedshot_combo_currentIndexChanged(int index);

    void on_weaponlist_combo_currentIndexChanged(int index);

    void on_players_tab_movespeed_valueChanged(double arg1);

    void on_damageModSpinBox_valueChanged(int arg1);

    void on_canDoubleJumpCheckBox_toggled(bool checked);

    void on_CanAirDashCheckBox_toggled(bool checked);

    void on_canShotDiagonal_toggled(bool checked);

    void on_playerFace_comboBox_currentIndexChanged(const QString &arg1);


    void on_weaponColor_picker_clicked();

    void on_simultaneousShots_spinBox_valueChanged(int arg1);

private:
    void fill_players_data();
    
private:
    Ui::player_edit *ui;
    bool _loading;
};

#endif // PLAYER_EDIT_H
