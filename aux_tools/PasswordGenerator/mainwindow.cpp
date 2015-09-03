#include "mainwindow.h"
#include "ui_mainwindow.h"



extern password_matrix matrix;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    CURRENT_FILE_FORMAT::file_io fio;

    fio.read_save(_save);
    ui->setupUi(this);

    password_generator pgen(_save);
    matrix = pgen.run();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_intro_checkBox_toggled(bool checked)
{
    set_stage_value(INTRO_STAGE, checked);
}

void MainWindow::set_stage_value(int stage, bool value)
{
    if (value == true) {
        _save.stages[stage] = 1;
    } else {
        _save.stages[stage] = 0;
    }
    regen_password();
}

void MainWindow::regen_password()
{
    password_generator pgen(_save);
    matrix = pgen.run();
    ui->widget->repaint();
}

void MainWindow::on_dynamite_checkBox_toggled(bool checked)
{
    set_stage_value(DYNAMITEBOT, checked);
}

void MainWindow::on_spike_checkBox_toggled(bool checked)
{
    set_stage_value(SPIKEBOT, checked);
}

void MainWindow::on_mummy_checkBox_toggled(bool checked)
{
    set_stage_value(MUMMYBOT, checked);
}

void MainWindow::on_seahorse_checkBox_toggled(bool checked)
{
    set_stage_value(SEAHORSEBOT, checked);
}

void MainWindow::on_techno_checkBox_toggled(bool checked)
{
    set_stage_value(TECHNOBOT, checked);
}

void MainWindow::on_daisie_checkBox_toggled(bool checked)
{
    set_stage_value(DAISIEBOT, checked);
}

void MainWindow::on_ape_checkBox_toggled(bool checked)
{
    set_stage_value(APEBOT, checked);
}

void MainWindow::on_mage_checkBox_toggled(bool checked)
{
    set_stage_value(MAGEBOT, checked);
}

void MainWindow::on_castle1_checkBox_toggled(bool checked)
{
    set_stage_value(SKULLCASTLE1, checked);
}

void MainWindow::on_castle2_checkBox_toggled(bool checked)
{
    set_stage_value(SKULLCASTLE2, checked);
}

void MainWindow::on_castle3_checkBox_toggled(bool checked)
{
    set_stage_value(SKULLCASTLE3, checked);
}

void MainWindow::on_castle4_checkBox_toggled(bool checked)
{
    set_stage_value(SKULLCASTLE4, checked);
}

void MainWindow::on_castle5_checkBox_toggled(bool checked)
{
    set_stage_value(SKULLCASTLE5, checked);
    // @TODO - game complete
}

void MainWindow::on_armor_arms_checkBox_toggled(bool checked)
{
    _save.armor_pieces[ARMOR_ARMS] = checked;
    regen_password();
}

void MainWindow::on_armor_body_checkBox_toggled(bool checked)
{
    _save.armor_pieces[ARMOR_BODY] = checked;
    regen_password();
}

void MainWindow::on_armor_legs_checkBox_toggled(bool checked)
{
    _save.armor_pieces[ARMOR_LEGS] = checked;
    regen_password();
}

void MainWindow::on_etank_spinBox_valueChanged(int arg1)
{
    _save.items.energy_tanks = arg1;
    regen_password();
}

void MainWindow::on_wtank_checkBox_toggled(bool checked)
{
    if (checked == true) {
        _save.items.weapon_tanks = 1;
    } else {
        _save.items.weapon_tanks = 0;
    }
    regen_password();
}

void MainWindow::on_stank_checkBox_toggled(bool checked)
{
    if (checked == true) {
        _save.items.special_tanks = 1;
    } else {
        _save.items.special_tanks = 0;
    }
    regen_password();
}
