#include "tab_text.h"
#include "ui_tab_text.h"
#include "../file/format.h"

extern std::vector<CURRENT_FILE_FORMAT::file_scene_sequence> sequences;
extern std::vector<CURRENT_FILE_FORMAT::file_scene> scenes;

TabText::TabText(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TabText)
{
    ui->setupUi(this);

    fill_data();
}

TabText::~TabText()
{
    delete ui;
}

void TabText::save_data()
{
    fio.save_scenes_show_text(ScenesMediator::get_instance()->text_list);
}

void TabText::fill_data()
{
    data_loading = true;

    ScenesMediator::get_instance()->text_list = fio.load_scenes_show_text();
    int list_size = ScenesMediator::get_instance()->text_list.size();

    if (list_size == 0) {
        change_fields_enabled(false);
    } else {
        // fill
        for (int i=0; i<list_size; i++) {
            ui->select_comboBox->addItem(QString(ScenesMediator::get_instance()->text_list.at(i).name));
        }
        set_fields(0);

    }
    data_loading = false;
    ui->select_comboBox->setCurrentIndex(0);
}

void TabText::set_fields(int index)
{
    ui->lineEdit1->setText(ScenesMediator::get_instance()->text_list.at(0).text_lines[0]);
    ui->lineEdit2->setText(ScenesMediator::get_instance()->text_list.at(0).text_lines[1]);
    ui->lineEdit3->setText(ScenesMediator::get_instance()->text_list.at(0).text_lines[2]);
    ui->lineEdit4->setText(ScenesMediator::get_instance()->text_list.at(0).text_lines[3]);
    ui->lineEdit5->setText(ScenesMediator::get_instance()->text_list.at(0).text_lines[4]);
    ui->lineEdit6->setText(ScenesMediator::get_instance()->text_list.at(0).text_lines[5]);
    ui->positionType->setCurrentIndex(ScenesMediator::get_instance()->text_list.at(0).position_type);
    change_x_y_fields_enabled();
    ui->textX_spinBox->setValue(ScenesMediator::get_instance()->text_list.at(0).x);
    ui->textY_spinBox->setValue(ScenesMediator::get_instance()->text_list.at(0).y);
    ui->transitionType_comboBox->setCurrentIndex(ScenesMediator::get_instance()->text_list.at(0).transition_type);
}

void TabText::on_AddButton_clicked()
{
    CURRENT_FILE_FORMAT::file_scene_show_text new_text;
    sprintf(new_text.name, "%s%d", "Show Text #", ScenesMediator::get_instance()->text_list.size()+1);
    ScenesMediator::get_instance()->text_list.push_back(new_text);
    ui->select_comboBox->addItem(QString(new_text.name));
    if (ScenesMediator::get_instance()->text_list.size() == 1) {
        change_fields_enabled(true);
    }
    ui->select_comboBox->setCurrentIndex(ScenesMediator::get_instance()->text_list.size()-1);
}

void TabText::change_fields_enabled(bool state)
{
    ui->lineEdit1->setEnabled(state);
    ui->lineEdit2->setEnabled(state);
    ui->lineEdit3->setEnabled(state);
    ui->lineEdit4->setEnabled(state);
    ui->lineEdit5->setEnabled(state);
    ui->lineEdit6->setEnabled(state);
    ui->positionType->setEnabled(state);
    ui->select_comboBox->setEnabled(state);
    ui->transitionType_comboBox->setEnabled(state);
    change_x_y_fields_enabled();
}

void TabText::change_x_y_fields_enabled()
{
    if (ui->positionType->isEnabled() == false) {
        ui->textX_spinBox->setEnabled(false);
        ui->textY_spinBox->setEnabled(false);
    } else {
        if (ui->positionType->currentIndex() == CURRENT_FILE_FORMAT::text_position_type_centered) {
            ui->textX_spinBox->setEnabled(false);
            ui->textY_spinBox->setEnabled(false);
        } else if (ui->positionType->currentIndex() == CURRENT_FILE_FORMAT::text_position_type_center_x) {
            ui->textX_spinBox->setEnabled(false);
            ui->textY_spinBox->setEnabled(true);
        } else if (ui->positionType->currentIndex() == CURRENT_FILE_FORMAT::text_position_type_center_y) {
            ui->textX_spinBox->setEnabled(true);
            ui->textY_spinBox->setEnabled(false);
        } else if (ui->positionType->currentIndex() == CURRENT_FILE_FORMAT::text_position_type_dialogbottom) {
            ui->textX_spinBox->setEnabled(false);
            ui->textY_spinBox->setEnabled(false);
        } else if (ui->positionType->currentIndex() == CURRENT_FILE_FORMAT::text_position_type_dialogtop) {
            ui->textX_spinBox->setEnabled(false);
            ui->textY_spinBox->setEnabled(false);
        } else if (ui->positionType->currentIndex() == CURRENT_FILE_FORMAT::text_position_type_user_defined) {
            ui->textX_spinBox->setEnabled(true);
            ui->textY_spinBox->setEnabled(true);
        }
    }
}



void TabText::on_positionType_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    int pos = ui->select_comboBox->currentIndex();
    ScenesMediator::get_instance()->text_list.at(pos).position_type = index;
    change_x_y_fields_enabled();
    ui->widget->repaint();
}

void TabText::on_lineEdit1_textChanged(const QString &arg1)
{
    if (data_loading) { return; }
    sprintf(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).text_lines[0], "%s", arg1.toStdString().c_str());
    ui->widget->repaint();
}

void TabText::on_lineEdit2_textChanged(const QString &arg1)
{
    if (data_loading) { return; }
    sprintf(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).text_lines[1], "%s", arg1.toStdString().c_str());
    ui->widget->repaint();
}

void TabText::on_lineEdit3_textChanged(const QString &arg1)
{
    if (data_loading) { return; }
    sprintf(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).text_lines[2], "%s", arg1.toStdString().c_str());
    ui->widget->repaint();
}

void TabText::on_lineEdit4_textChanged(const QString &arg1)
{
    if (data_loading) { return; }
    sprintf(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).text_lines[3], "%s", arg1.toStdString().c_str());
    ui->widget->repaint();
}

void TabText::on_lineEdit5_textChanged(const QString &arg1)
{
    if (data_loading) { return; }
    sprintf(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).text_lines[4], "%s", arg1.toStdString().c_str());
    ui->widget->repaint();
}

void TabText::on_lineEdit6_textChanged(const QString &arg1)
{
    if (data_loading) { return; }
    sprintf(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).text_lines[5], "%s", arg1.toStdString().c_str());
    ui->widget->repaint();
}

void TabText::on_select_comboBox_currentIndexChanged(int index)
{
    data_loading = true;
    ui->lineEdit1->setText(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).text_lines[0]);
    ui->lineEdit2->setText(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).text_lines[1]);
    ui->lineEdit3->setText(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).text_lines[2]);
    ui->lineEdit4->setText(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).text_lines[3]);
    ui->lineEdit5->setText(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).text_lines[4]);
    ui->lineEdit6->setText(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).text_lines[5]);
    ui->positionType->setCurrentIndex(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).position_type);
    ui->textX_spinBox->setValue(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).x);
    ui->textY_spinBox->setValue(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).y);
    ui->transitionType_comboBox->setCurrentIndex(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).transition_type);
    change_x_y_fields_enabled();
    data_loading = false;
    ui->widget->set_selected_n(index);
    ui->widget->repaint();
}
