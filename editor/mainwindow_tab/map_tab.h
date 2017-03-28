#ifndef MAP_TAB_H
#define MAP_TAB_H

#include <QWidget>

#include "file/file_io.h"

namespace Ui {
class map_tab;
}

class map_tab : public QWidget
{
    Q_OBJECT
    
public:
    explicit map_tab(QWidget *parent = 0);
    ~map_tab();
    void reload();
    void set_current_box(short n);
    void update_edit_area();

public slots:
    void on_color_selected1(QColor);

private slots:
    void on_stageListCombo_currentIndexChanged(int index);
    void on_mapListCombo_currentIndexChanged(int index);
    void on_comboBox_currentIndexChanged(int index);
    void on_listWidget_2_currentRowChanged(int currentRow);
    void on_npc_listWidget_currentRowChanged(int currentRow);
    void on_npc_direction_combo_currentIndexChanged(int index);
    void on_link_orientation_combobox_currentIndexChanged(int index);
    void on_link_bidi_checkbox_toggled(bool checked);
    void on_link_isdoor_checkbox_toggled(bool checked);
    void on_objectListWidget_currentRowChanged(int currentRow);
    void on_bg_color_pick_clicked();
    void on_bg1_filename_currentIndexChanged(const QString &arg1);
    void on_bg1_speed_valueChanged(double arg1);
    void on_bg1_y_pos_valueChanged(int arg1);
    void on_checkBox_toggled(bool checked);
    void on_checkBox_2_toggled(bool checked);
    void on_object_direction_combo_currentIndexChanged(int index);
    void on_editTile_button_clicked();
    void on_editObject_button_clicked();
    void on_editLink_button_clicked();
    void on_editNpc_button_clicked();
    void on_editSetSubBoss_button_clicked();
    void on_editSetBoss_button_clicked();
    void on_editModeNormal_button_clicked();
    void on_editModeLock_button_clicked();
    void on_editModeErase_button_clicked();
    void on_stageTileset_comboBox_currentIndexChanged(const QString &arg1);

    void on_mapGFX_comboBox_currentIndexChanged(int index);

    void on_autoScrollBG1_mode_currentIndexChanged(int index);

    void on_addAnimTile_toolButton_clicked();

    void on_toolButton_clicked();

    void on_paste_toolButton_clicked();

    void on_mapAutoScroll_checkBox_clicked(bool checked);

    void on_fb_image_comboBox_currentIndexChanged(const QString &arg1);

    void on_fg_speed_doubleSpinBox_valueChanged(double arg1);

    void on_fg_position_spinBox_valueChanged(int arg1);

    void on_fg_show_checkBox_toggled(bool checked);

    void on_fg_opacity_spinBox_valueChanged(int arg1);

    void on_mapGFXMode_comboBox_currentIndexChanged(int index);

private:
    void fill_data();
    void fill_background_list();
    void fill_anim_tiles_data();
    
private:
    Ui::map_tab *ui;
    bool _data_loading;
    CURRENT_FILE_FORMAT::file_io fio;
};

#endif // MAP_TAB_H
