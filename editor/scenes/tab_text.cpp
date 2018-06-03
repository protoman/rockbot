#include "tab_text.h"
#include "ui_tab_text.h"
#include "../file/format.h"
#include "common.h"

TabText::TabText(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TabText)
{
    dataLoading = true;
    currentIndex = -1;
    currentLanguage = LANGUAGE_ENGLISH;

    ui->setupUi(this);

    common::fill_languages_combo(ui->languageComboBox);

    fill_data();
    dataLoading = false;
}

TabText::~TabText()
{
    delete ui;
}

void TabText::save_data()
{
    fio_scenes.save_scenes_show_text(ScenesMediator::get_instance()->text_list);

    save_data(currentIndex);
}

void TabText::save_data(int n)
{
    std::vector<std::string> text_list;
    for (int i=0; i<SCENE_TEXT_LINES_N; i++) {
        text_list.push_back(scene_text_list[i]);
    }
    fio_str.write_scene_text_file(currentIndex, text_list, currentLanguage);
}

void TabText::reload()
{
    fill_data();
}

void TabText::fill_data()
{
    data_loading = true;


    ScenesMediator::get_instance()->text_list = fio_scenes.load_scenes_show_text();
    int list_size = ScenesMediator::get_instance()->text_list.size();

    if (list_size == 0) {
        change_fields_enabled(false);
    } else {
        change_fields_enabled(true);
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
    std::vector<std::string> text_list = fio_str.get_string_list_from_scene_text_file(index, currentLanguage);
    for (int i=0; i<text_list.size(); i++) {
        scene_text_list[i] = text_list.at(i);
    }

    ui->lineEdit1->setText(scene_text_list[0].c_str());
    ui->lineEdit2->setText(scene_text_list[1].c_str());
    ui->lineEdit3->setText(scene_text_list[2].c_str());
    ui->lineEdit4->setText(scene_text_list[3].c_str());
    ui->lineEdit5->setText(scene_text_list[4].c_str());
    ui->lineEdit6->setText(scene_text_list[5].c_str());

    ui->positionType->setCurrentIndex(ScenesMediator::get_instance()->text_list.at(index).position_type);
    change_x_y_fields_enabled();
    ui->textX_spinBox->setValue(ScenesMediator::get_instance()->text_list.at(index).x);
    ui->textY_spinBox->setValue(ScenesMediator::get_instance()->text_list.at(index).y);
    ui->transitionType_comboBox->setCurrentIndex(ScenesMediator::get_instance()->text_list.at(index).transition_type);
    ui->name_lineEdit->setText(ScenesMediator::get_instance()->text_list.at(index).name);
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

void TabText::on_select_comboBox_currentIndexChanged(int index)
{
    // save previous entry
    if (data_loading == false && currentIndex != -1) {
        save_data(currentIndex);
        std::cout << "SAVING..." << std::endl;
    }

    data_loading = true;

    set_fields(index);
    currentIndex = index;

    data_loading = false;

    ui->name_lineEdit->setText(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).name);
    ui->widget->set_selected_n(index, scene_text_list);
    ui->widget->repaint();
}


void TabText::on_name_lineEdit_textChanged(const QString &arg1)
{
    if (data_loading == true) { return; }
    sprintf(ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).name, "%s", arg1.toStdString().c_str());
}


void TabText::on_textX_spinBox_valueChanged(int arg1)
{
    if (data_loading == true) { return; }
    ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).x = arg1;
    ui->widget->set_selected_n(ui->select_comboBox->currentIndex(), scene_text_list);
    ui->widget->repaint();
}

void TabText::on_textY_spinBox_valueChanged(int arg1)
{
    if (data_loading == true) { return; }
    ScenesMediator::get_instance()->text_list.at(ui->select_comboBox->currentIndex()).y = arg1;
    ui->widget->repaint();
}

void TabText::on_lineEdit1_textChanged(const QString &arg1)
{
    if (data_loading == true) { return; }
    scene_text_list[0] = arg1.toStdString();
    ui->widget->set_selected_n(ui->select_comboBox->currentIndex(), scene_text_list);
    ui->widget->repaint();
}

void TabText::on_lineEdit2_textChanged(const QString &arg1)
{
    if (data_loading == true) { return; }
    scene_text_list[1] = arg1.toStdString();
    ui->widget->set_selected_n(ui->select_comboBox->currentIndex(), scene_text_list);
    ui->widget->repaint();
}

void TabText::on_lineEdit3_textChanged(const QString &arg1)
{
    if (data_loading == true) { return; }
    scene_text_list[2] = arg1.toStdString();
    ui->widget->set_selected_n(ui->select_comboBox->currentIndex(), scene_text_list);
    ui->widget->repaint();
}

void TabText::on_lineEdit4_textChanged(const QString &arg1)
{
    if (data_loading == true) { return; }
    scene_text_list[3] = arg1.toStdString();
    ui->widget->set_selected_n(ui->select_comboBox->currentIndex(), scene_text_list);
    ui->widget->repaint();
}

void TabText::on_lineEdit5_textChanged(const QString &arg1)
{
    if (data_loading == true) { return; }
    scene_text_list[4] = arg1.toStdString();
    ui->widget->set_selected_n(ui->select_comboBox->currentIndex(), scene_text_list);
    ui->widget->repaint();
}

void TabText::on_lineEdit6_textChanged(const QString &arg1)
{
    if (data_loading == true) { return; }
    scene_text_list[5] = arg1.toStdString();
    ui->widget->set_selected_n(ui->select_comboBox->currentIndex(), scene_text_list);
    ui->widget->repaint();
}

void TabText::on_languageComboBox_currentIndexChanged(int index)
{
    if (dataLoading == true) {
        return;
    }
    save_data();
    currentLanguage = index;
    set_fields(currentIndex);
}
