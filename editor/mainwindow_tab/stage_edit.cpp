#include "stage_edit.h"
#include "ui_stage_edit.h"

//#include "defines.h"
//#include "../defines.h"
#include "common.h"

#include "../mediator.h"

stage_edit::stage_edit(QWidget *parent) : QWidget(parent), ui(new Ui::stage_edit), _data_loading(true)
{
	ui->setupUi(this);
    ui->dialogs_line1_text1->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->dialogs_line1_text2->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->dialogs_line1_text3->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->dialogs_answer1_text1->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->dialogs_answer1_text2->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->dialogs_answer1_text3->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);

    ui->dialogs_line2_text1->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->dialogs_line2_text2->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->dialogs_line2_text3->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->dialogs_answer2_text1->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->dialogs_answer2_text2->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->dialogs_answer2_text3->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);

    ui->boss_dialog_text1_line1->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->boss_dialog_text1_line2->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->boss_dialog_text1_line3->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->boss_dialog_answer1_line1->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->boss_dialog_answer1_line2->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->boss_dialog_answer1_line3->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);

    ui->boss_dialog_text2_line1->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->boss_dialog_text2_line2->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->boss_dialog_text2_line3->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->boss_dialog_answer2_line1->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->boss_dialog_answer2_line2->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);
    ui->boss_dialog_answer2_line3->setMaxLength(EDITOR_DIALOG_LINE_LIMIT);

    Mediator::get_instance()->currentStage = INTRO_STAGE;
	fill_stage_tab_data();
    _data_loading = false;
}

stage_edit::~stage_edit()
{
    delete ui;
}

void stage_edit::reload()
{
    Mediator::get_instance()->currentStage = INTRO_STAGE;
    fill_stage_tab_data();
}



void stage_edit::fill_stage_tab_data()
{
    if (FILEPATH.length() == 0) {
        return;
    }
    _data_loading = true;
    QListWidgetItem item_empty;
    item_empty.setText("");
    std::string filename = FILEPATH + "/images/empty.png";
	QPixmap image(filename.c_str());
    if (image.isNull() == false && image.width() > 0) {
        image = image.copy(0, 0, image.width(), image.height());
        image = image.scaled(32, 32);
        item_empty.setIcon(image);
    }

    Mediator::get_instance()->current_player = 0;


    common::fill_players_combo(ui->dialogs_answer1_player);

	// FILL ITEMS
    common::fill_stages_combo(ui->stages_tab_stage_combo);
	// fill bg music combo
    common::fill_files_combo(std::string("/music"), ui->stages_tab_bgmusic_combo);
	// fill boss faces
    common::fill_graphicfiles_combobox(std::string("/images/faces"), ui->bossface_comboBox);
	// fill digalogs faces
    common::fill_graphicfiles_combobox(std::string("/images/faces"), ui->dialogs_line1_face_combo);

    common::fill_weapons_combo(ui->stage_boss_weapon_combo);
    ui->stage_boss_weapon_combo->setCurrentIndex(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.id_weapon);

	update_stage_data();
    _data_loading = false;
}


void stage_edit::update_stage_data()
{
    if (Mediator::get_instance()->currentStage < 0) {
        return;
    }

	// SET ITEMS
    ui->stages_tab_stage_name_lineedit->setText(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].name);
    ui->stages_tab_bossname_lineedit->setText(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.name);
    int combo_n = ui->stages_tab_bgmusic_combo->findText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].bgmusic_filename));
    //std::cout << "&&&&& currentStage: " << Mediator::get_instance()->currentStage << ", music-file: '" << Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].bgmusic_filename << "'', combo_n: " << combo_n << std::endl;
    ui->stages_tab_bgmusic_combo->setCurrentIndex(combo_n);
    //ui->dialogs_line1_face->setCurrentIndex(ui->dialogs_line1_face->findText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.face_graphics_filename)));

    // search for the item that matches the text to select row
    std::string search_text = std::string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.face_graphics_filename);
    for (int i=0; i<ui->dialogs_line1_face_combo->count(); i++) {
        int combo_n = ui->dialogs_line1_face_combo->findText(QString(search_text.c_str()));
        ui->dialogs_line1_face_combo->setCurrentIndex(combo_n);
        break;
    }

    search_text = std::string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.face_graphics_filename);
    for (int i=0; i<ui->bossface_comboBox->count(); i++) {
        int combo_n = ui->bossface_comboBox->findText(QString(search_text.c_str()));
        ui->bossface_comboBox->setCurrentIndex(combo_n);
        break;
    }

    //std::cout << "stage: " << Mediator::get_instance()->currentStage << ", boss.id_weapon: " << Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.id_weapon << std::endl;
    ui->stage_boss_weapon_combo->setCurrentIndex(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.id_weapon);

    ui->dialogs_line1_text1->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.line1[0]));
    ui->dialogs_line1_text2->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.line1[1]));
    ui->dialogs_line1_text3->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.line1[2]));

    ui->dialogs_line2_text1->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.line2[0]));
    ui->dialogs_line2_text2->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.line2[1]));
    ui->dialogs_line2_text3->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.line2[2]));

    ui->dialogs_answer1_text1->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer1[Mediator::get_instance()->current_player][0]));
    ui->dialogs_answer1_text2->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer1[Mediator::get_instance()->current_player][1]));
    ui->dialogs_answer1_text3->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer1[Mediator::get_instance()->current_player][2]));

    ui->dialogs_answer2_text1->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer2[Mediator::get_instance()->current_player][0]));
    ui->dialogs_answer2_text2->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer2[Mediator::get_instance()->current_player][1]));
    ui->dialogs_answer2_text3->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer2[Mediator::get_instance()->current_player][2]));



    ui->boss_dialog_text1_line1->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.line1[0]));
    ui->boss_dialog_text1_line2->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.line1[1]));
    ui->boss_dialog_text1_line3->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.line1[2]));

    ui->boss_dialog_text2_line1->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.line2[0]));
    ui->boss_dialog_text2_line2->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.line2[1]));
    ui->boss_dialog_text2_line3->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.line2[2]));

    ui->boss_dialog_answer1_line1->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer1[Mediator::get_instance()->current_player][0]));
    ui->boss_dialog_answer1_line2->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer1[Mediator::get_instance()->current_player][1]));
    ui->boss_dialog_answer1_line3->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer1[Mediator::get_instance()->current_player][2]));

    ui->boss_dialog_answer2_line1->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer2[Mediator::get_instance()->current_player][0]));
    ui->boss_dialog_answer2_line2->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer2[Mediator::get_instance()->current_player][1]));
    ui->boss_dialog_answer2_line3->setText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer2[Mediator::get_instance()->current_player][2]));


}

void stage_edit::on_stages_tab_stage_combo_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->currentStage = index;
	update_stage_data();
}

void stage_edit::on_stages_tab_bossfaces_view_itemClicked(QListWidgetItem *item)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.face_graphics_filename, "%s", item->text().toStdString().c_str());
}



void stage_edit::on_dialogs_line1_text1_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.line1[0], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_line1_text2_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.line1[1], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_line1_text3_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.line1[2], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_line2_text1_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.line2[0], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_line2_text2_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.line2[1], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_line2_text3_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.line2[2], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_answer1_text1_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer1[Mediator::get_instance()->current_player][0], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_answer1_text2_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer1[Mediator::get_instance()->current_player][1], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_answer1_text3_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer1[Mediator::get_instance()->current_player][2], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_answer2_text1_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer2[Mediator::get_instance()->current_player][0], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_answer2_text2_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer2[Mediator::get_instance()->current_player][1], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_answer2_text3_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer2[Mediator::get_instance()->current_player][2], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_answer1_player_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->current_player = index;
    update_stage_data();
}

void stage_edit::on_boss_dialog_text1_line1_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.line1[0], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_text1_line2_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.line1[1], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_text1_line3_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.line1[2], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_answer1_line1_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer1[Mediator::get_instance()->current_player][0], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_answer1_line2_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer1[Mediator::get_instance()->current_player][1], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_answer1_line3_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer1[Mediator::get_instance()->current_player][2], "%s", arg1.toStdString().c_str());
}


void stage_edit::on_boss_dialog_text2_line1_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.line2[0], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_text2_line2_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.line2[1], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_text2_line3_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.line2[2], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_answer2_line1_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer2[Mediator::get_instance()->current_player][0], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_answer2_line2_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer2[Mediator::get_instance()->current_player][1], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_answer2_line3_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer2[Mediator::get_instance()->current_player][2], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_stages_tab_bgmusic_combo_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].bgmusic_filename, "%s", arg1.toStdString().c_str());
}

void stage_edit::on_stages_tab_stage_name_lineedit_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].name, "%s", arg1.toStdString().c_str());
}

void stage_edit::on_stages_tab_bossname_lineedit_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.name, "%s", arg1.toStdString().c_str());
}




void stage_edit::on_dialogs_line1_face_combo_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.face_graphics_filename, "%s", arg1.toStdString().c_str());
}

void stage_edit::on_bossface_comboBox_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) return;

    sprintf(Mediator::get_instance()->game_data.stage_face_filename[Mediator::get_instance()->currentStage], "%s", arg1.toStdString().c_str()); // this is used to have faces without loading the whole stage
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.face_graphics_filename, "%s", arg1.toStdString().c_str());
}
