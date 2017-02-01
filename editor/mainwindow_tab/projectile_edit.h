#ifndef PROJECTILE_EDIT_H
#define PROJECTILE_EDIT_H

#include <QWidget>

namespace Ui {
class projectile_edit;
}

class projectile_edit : public QWidget
{
    Q_OBJECT
    
public:
    explicit projectile_edit(QWidget *parent = 0);
    ~projectile_edit();
    void reload();

private slots:
    void on_projectileList_combo_currentIndexChanged(int index);

	void on_name_textChanged(const QString &arg1);

	void on_graphic_filename_currentIndexChanged(const QString &arg1);

	void on_trajectory_currentIndexChanged(int index);

	void on_img_w_valueChanged(int arg1);

	void on_img_h_valueChanged(int arg1);

	void on_projectileDestructibleCheckBox_toggled(bool checked);

	void on_projectileHitPointsSpinBox_valueChanged(int arg1);

	void on_max_shots_valueChanged(int arg1);

	void on_speed_valueChanged(int arg1);

	void on_damage_valueChanged(int arg1);

    void on_pushButton_clicked();

    void on_sfxFilename_comboBox_currentIndexChanged(const QString &arg1);

    void on_explosive_checkBox_toggled(bool checked);

private:
    void fill_data();
    void set_edit_data(int index);
    
private:
    Ui::projectile_edit *ui;
    bool data_loading;
};

#endif // PROJECTILE_EDIT_H
