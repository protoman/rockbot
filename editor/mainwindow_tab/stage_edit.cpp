#include "stage_edit.h"
#include "ui_stage_edit.h"

//#include "defines.h"
//#include "../defines.h"
#include "common.h"

#include "../mediator.h"
extern Mediator *dataExchanger;
extern char EDITOR_FILEPATH[512];
extern std::string FILEPATH;

extern CURRENT_FILE_FORMAT::file_stages stage_data;

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

    dataExchanger->currentStage = INTRO_STAGE;
	fill_stage_tab_data();
    _data_loading = false;
}

stage_edit::~stage_edit()
{
    delete ui;
}

void stage_edit::reload()
{
    dataExchanger->currentStage = INTRO_STAGE;
    std::cout << "#A - STAGE_N: " << dataExchanger->currentStage << std::endl;
    fill_stage_tab_data();
}



void stage_edit::fill_stage_tab_data()
{
    std::cout << "#B - STAGE_N: " << dataExchanger->currentStage << std::endl;
    _data_loading = true;
    QListWidgetItem item_empty;
    item_empty.setText("");
	std::string filename = FILEPATH + "/data/images/empty.png";
	QPixmap image(filename.c_str());
    if (image.isNull() == false && image.width() > 0) {
        image = image.copy(0, 0, image.width(), image.height());
        image = image.scaled(32, 32);
        item_empty.setIcon(image);
    }

	dataExchanger->current_player = 0;


    common::fill_players_combo(ui->dialogs_answer1_player);

	// FILL ITEMS
    common::fill_stages_combo(ui->stages_tab_stage_combo);
	// fill bg music combo
	common::fill_files_combo(std::string("/data/music"), ui->stages_tab_bgmusic_combo);
	// fill boss faces
    common::fill_graphicfiles_combobox(std::string("data/images/faces"), ui->bossface_comboBox);
	// fill digalogs faces
    common::fill_graphicfiles_combobox(std::string("data/images/faces"), ui->dialogs_line1_face_combo);

    common::fill_weapons_combo(ui->stage_boss_weapon_combo);
    ui->stage_boss_weapon_combo->setCurrentIndex(stage_data.stages[dataExchanger->currentStage].boss.id_weapon);

	update_stage_data();
    _data_loading = false;
}


void stage_edit::update_stage_data()
{
    if (dataExchanger->currentStage < 0) {
        return;
    }
    std::cout << "#1 - STAGE_N: " << dataExchanger->currentStage << std::endl;

	// SET ITEMS
    ui->stages_tab_stage_name_lineedit->setText(stage_data.stages[dataExchanger->currentStage].name);
    ui->stages_tab_bossname_lineedit->setText(stage_data.stages[dataExchanger->currentStage].boss.name);
    int combo_n = ui->stages_tab_bgmusic_combo->findText(QString(stage_data.stages[dataExchanger->currentStage].bgmusic_filename));
    //std::cout << "&&&&& currentStage: " << dataExchanger->currentStage << ", music-file: '" << stage_data.stages[dataExchanger->currentStage].bgmusic_filename << "'', combo_n: " << combo_n << std::endl;
    ui->stages_tab_bgmusic_combo->setCurrentIndex(combo_n);
    //ui->dialogs_line1_face->setCurrentIndex(ui->dialogs_line1_face->findText(QString(stage_data.stages[dataExchanger->currentStage].intro_dialog.face_graphics_filename)));

    // search for the item that matches the text to select row
    std::string search_text = std::string(stage_data.stages[dataExchanger->currentStage].intro_dialog.face_graphics_filename);
    for (int i=0; i<ui->dialogs_line1_face_combo->count(); i++) {
        int combo_n = ui->dialogs_line1_face_combo->findText(QString(search_text.c_str()));
        ui->dialogs_line1_face_combo->setCurrentIndex(combo_n);
        break;
    }

    search_text = std::string(stage_data.stages[dataExchanger->currentStage].boss.face_graphics_filename);
    for (int i=0; i<ui->bossface_comboBox->count(); i++) {
        int combo_n = ui->bossface_comboBox->findText(QString(search_text.c_str()));
        ui->bossface_comboBox->setCurrentIndex(combo_n);
        break;
    }

    //std::cout << "stage: " << dataExchanger->currentStage << ", boss.id_weapon: " << stage_data.stages[dataExchanger->currentStage].boss.id_weapon << std::endl;
    ui->stage_boss_weapon_combo->setCurrentIndex(stage_data.stages[dataExchanger->currentStage].boss.id_weapon);

    ui->dialogs_line1_text1->setText(QString(stage_data.stages[dataExchanger->currentStage].intro_dialog.line1[0]));
    ui->dialogs_line1_text2->setText(QString(stage_data.stages[dataExchanger->currentStage].intro_dialog.line1[1]));
    ui->dialogs_line1_text3->setText(QString(stage_data.stages[dataExchanger->currentStage].intro_dialog.line1[2]));

    ui->dialogs_line2_text1->setText(QString(stage_data.stages[dataExchanger->currentStage].intro_dialog.line2[0]));
    ui->dialogs_line2_text2->setText(QString(stage_data.stages[dataExchanger->currentStage].intro_dialog.line2[1]));
    ui->dialogs_line2_text3->setText(QString(stage_data.stages[dataExchanger->currentStage].intro_dialog.line2[2]));

    ui->dialogs_answer1_text1->setText(QString(stage_data.stages[dataExchanger->currentStage].intro_dialog.answer1[dataExchanger->current_player][0]));
    ui->dialogs_answer1_text2->setText(QString(stage_data.stages[dataExchanger->currentStage].intro_dialog.answer1[dataExchanger->current_player][1]));
    ui->dialogs_answer1_text3->setText(QString(stage_data.stages[dataExchanger->currentStage].intro_dialog.answer1[dataExchanger->current_player][2]));

    ui->dialogs_answer2_text1->setText(QString(stage_data.stages[dataExchanger->currentStage].intro_dialog.answer2[dataExchanger->current_player][0]));
    ui->dialogs_answer2_text2->setText(QString(stage_data.stages[dataExchanger->currentStage].intro_dialog.answer2[dataExchanger->current_player][1]));
    ui->dialogs_answer2_text3->setText(QString(stage_data.stages[dataExchanger->currentStage].intro_dialog.answer2[dataExchanger->current_player][2]));



    ui->boss_dialog_text1_line1->setText(QString(stage_data.stages[dataExchanger->currentStage].boss_dialog.line1[0]));
    ui->boss_dialog_text1_line2->setText(QString(stage_data.stages[dataExchanger->currentStage].boss_dialog.line1[1]));
    ui->boss_dialog_text1_line3->setText(QString(stage_data.stages[dataExchanger->currentStage].boss_dialog.line1[2]));

    ui->boss_dialog_text2_line1->setText(QString(stage_data.stages[dataExchanger->currentStage].boss_dialog.line2[0]));
    ui->boss_dialog_text2_line2->setText(QString(stage_data.stages[dataExchanger->currentStage].boss_dialog.line2[1]));
    ui->boss_dialog_text2_line3->setText(QString(stage_data.stages[dataExchanger->currentStage].boss_dialog.line2[2]));

    ui->boss_dialog_answer1_line1->setText(QString(stage_data.stages[dataExchanger->currentStage].boss_dialog.answer1[dataExchanger->current_player][0]));
    ui->boss_dialog_answer1_line2->setText(QString(stage_data.stages[dataExchanger->currentStage].boss_dialog.answer1[dataExchanger->current_player][1]));
    ui->boss_dialog_answer1_line3->setText(QString(stage_data.stages[dataExchanger->currentStage].boss_dialog.answer1[dataExchanger->current_player][2]));

    ui->boss_dialog_answer2_line1->setText(QString(stage_data.stages[dataExchanger->currentStage].boss_dialog.answer2[dataExchanger->current_player][0]));
    ui->boss_dialog_answer2_line2->setText(QString(stage_data.stages[dataExchanger->currentStage].boss_dialog.answer2[dataExchanger->current_player][1]));
    ui->boss_dialog_answer2_line3->setText(QString(stage_data.stages[dataExchanger->currentStage].boss_dialog.answer2[dataExchanger->current_player][2]));


}

void stage_edit::on_stages_tab_stage_combo_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    std::cout << "#2 - STAGE_N: " << index << std::endl;
	dataExchanger->currentStage = index;
	update_stage_data();
}

void stage_edit::on_stages_tab_bossfaces_view_itemClicked(QListWidgetItem *item)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].boss.face_graphics_filename, "%s", item->text().toStdString().c_str());
}



void stage_edit::on_dialogs_line1_text1_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].intro_dialog.line1[0], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_line1_text2_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].intro_dialog.line1[1], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_line1_text3_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].intro_dialog.line1[2], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_line2_text1_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].intro_dialog.line2[0], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_line2_text2_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].intro_dialog.line2[1], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_line2_text3_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].intro_dialog.line2[2], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_answer1_text1_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].intro_dialog.answer1[dataExchanger->current_player][0], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_answer1_text2_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].intro_dialog.answer1[dataExchanger->current_player][1], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_answer1_text3_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].intro_dialog.answer1[dataExchanger->current_player][2], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_answer2_text1_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].intro_dialog.answer2[dataExchanger->current_player][0], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_answer2_text2_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].intro_dialog.answer2[dataExchanger->current_player][1], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_answer2_text3_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].intro_dialog.answer2[dataExchanger->current_player][2], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_dialogs_answer1_player_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
	dataExchanger->current_player = index;
	update_stage_data();
}

void stage_edit::on_boss_dialog_text1_line1_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].boss_dialog.line1[0], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_text1_line2_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].boss_dialog.line1[1], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_text1_line3_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].boss_dialog.line1[2], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_answer1_line1_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].boss_dialog.answer1[dataExchanger->current_player][0], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_answer1_line2_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].boss_dialog.answer1[dataExchanger->current_player][1], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_answer1_line3_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].boss_dialog.answer1[dataExchanger->current_player][2], "%s", arg1.toStdString().c_str());
}


void stage_edit::on_boss_dialog_text2_line1_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].boss_dialog.line2[0], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_text2_line2_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].boss_dialog.line2[1], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_text2_line3_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].boss_dialog.line2[2], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_answer2_line1_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].boss_dialog.answer2[dataExchanger->current_player][0], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_answer2_line2_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].boss_dialog.answer2[dataExchanger->current_player][1], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_boss_dialog_answer2_line3_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].boss_dialog.answer2[dataExchanger->current_player][2], "%s", arg1.toStdString().c_str());
}

void stage_edit::on_stages_tab_bgmusic_combo_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].bgmusic_filename, "%s", arg1.toStdString().c_str());
}

void stage_edit::on_stages_tab_stage_name_lineedit_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].name, "%s", arg1.toStdString().c_str());
}

void stage_edit::on_stages_tab_bossname_lineedit_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].boss.name, "%s", arg1.toStdString().c_str());
}




void stage_edit::on_dialogs_line1_face_combo_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(stage_data.stages[dataExchanger->currentStage].intro_dialog.face_graphics_filename, "%s", arg1.toStdString().c_str());
}

void stage_edit::on_bossface_comboBox_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) return;

    sprintf(game_data.stage_face_filename[dataExchanger->currentStage], "%s", arg1.toStdString().c_str()); // this is used to have faces without loading the whole stage
    sprintf(stage_data.stages[dataExchanger->currentStage].boss.face_graphics_filename, "%s", arg1.toStdString().c_str());
}
