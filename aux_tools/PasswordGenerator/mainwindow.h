#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "../../defines.h"
#include "../../file/version.h"
#include "../../file/file_io.h"
#include "../../scenes/password_generator.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_intro_checkBox_toggled(bool checked);

    void on_dynamite_checkBox_toggled(bool checked);

    void on_spike_checkBox_toggled(bool checked);

    void on_mummy_checkBox_toggled(bool checked);

    void on_seahorse_checkBox_toggled(bool checked);

    void on_techno_checkBox_toggled(bool checked);

    void on_daisie_checkBox_toggled(bool checked);

    void on_ape_checkBox_toggled(bool checked);

    void on_mage_checkBox_toggled(bool checked);

    void on_castle1_checkBox_toggled(bool checked);

    void on_castle2_checkBox_toggled(bool checked);

    void on_castle3_checkBox_toggled(bool checked);

    void on_castle4_checkBox_toggled(bool checked);

    void on_castle5_checkBox_toggled(bool checked);

    void on_armor_arms_checkBox_toggled(bool checked);

    void on_armor_body_checkBox_toggled(bool checked);

    void on_armor_legs_checkBox_toggled(bool checked);

    void on_etank_spinBox_valueChanged(int arg1);

    void on_wtank_checkBox_toggled(bool checked);

    void on_stank_checkBox_toggled(bool checked);

private:
    void set_stage_value(int stage, bool value);
    void regen_password();

private:
    Ui::MainWindow *ui;
    CURRENT_FILE_FORMAT::st_save _save;
};

#endif // MAINWINDOW_H
