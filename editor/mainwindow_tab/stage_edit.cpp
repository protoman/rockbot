#include "stage_edit.h"
#include "ui_stage_edit.h"

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
	// fill digalogs faces
    common::fill_graphicfiles_combobox(std::string("/images/faces"), ui->dialogs_line1_face_combo);

    common::fill_weapons_combo(ui->stage_boss_weapon_combo);
    ui->stage_boss_weapon_combo->setCurrentIndex(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.id_weapon);

    common::fill_scenes_combo(ui->cutscenePre_comboBox);

    common::fill_scenes_combo(ui->cutscenePos_comboBox);

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

    //std::cout << "stage: " << Mediator::get_instance()->currentStage << ", boss.id_weapon: " << Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.id_weapon << std::endl;
    ui->stage_boss_weapon_combo->setCurrentIndex(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.id_weapon);



    ui->dialogs_line1_text1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.text1_string_ids[0]).c_str()));
    ui->dialogs_line1_text2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.text1_string_ids[1]).c_str()));
    ui->dialogs_line1_text3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.text1_string_ids[2]).c_str()));

    ui->dialogs_line2_text1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.text2_string_ids[0]).c_str()));
    ui->dialogs_line2_text2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.text2_string_ids[1]).c_str()));
    ui->dialogs_line2_text3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.text2_string_ids[2]).c_str()));

    ui->dialogs_answer1_text1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer1_string_ids[ui->dialogs_answer1_player->currentIndex()][0]).c_str()));
    ui->dialogs_answer1_text2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer1_string_ids[ui->dialogs_answer1_player->currentIndex()][1]).c_str()));
    ui->dialogs_answer1_text3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer1_string_ids[ui->dialogs_answer1_player->currentIndex()][2]).c_str()));

    ui->dialogs_answer2_text1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer2_string_ids[ui->dialogs_answer1_player->currentIndex()][0]).c_str()));
    ui->dialogs_answer2_text2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer2_string_ids[ui->dialogs_answer1_player->currentIndex()][1]).c_str()));
    ui->dialogs_answer2_text3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer2_string_ids[ui->dialogs_answer1_player->currentIndex()][2]).c_str()));


    ui->boss_dialog_text1_line1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.text1_string_ids[0]).c_str()));
    ui->boss_dialog_text1_line2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.text1_string_ids[1]).c_str()));
    ui->boss_dialog_text1_line3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.text1_string_ids[2]).c_str()));

    ui->boss_dialog_text2_line1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.text2_string_ids[0]).c_str()));
    ui->boss_dialog_text2_line2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.text2_string_ids[1]).c_str()));
    ui->boss_dialog_text2_line3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.text2_string_ids[2]).c_str()));

    ui->boss_dialog_answer1_line1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer1_string_ids[ui->dialogs_answer1_player->currentIndex()][0]).c_str()));
    ui->boss_dialog_answer1_line2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer1_string_ids[ui->dialogs_answer1_player->currentIndex()][1]).c_str()));
    ui->boss_dialog_answer1_line3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer1_string_ids[ui->dialogs_answer1_player->currentIndex()][2]).c_str()));

    ui->boss_dialog_answer2_line1->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer2_string_ids[ui->dialogs_answer1_player->currentIndex()][0]).c_str()));
    ui->boss_dialog_answer2_line2->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer2_string_ids[ui->dialogs_answer1_player->currentIndex()][1]).c_str()));
    ui->boss_dialog_answer2_line3->setText(QString(fio_str.get_common_string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer2_string_ids[ui->dialogs_answer1_player->currentIndex()][2]).c_str()));

    ui->cutscenePre_comboBox->setCurrentIndex(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].cutscene_pre);
    ui->cutscenePos_comboBox->setCurrentIndex(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].cutscene_pos);

    ui->stages_tab_bossname_lineedit->setText(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.name);
}


void stage_edit::on_stages_tab_stage_combo_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->currentStage = index;
	update_stage_data();
}


void stage_edit::on_dialogs_answer1_player_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->current_player = index;
    update_stage_data();
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


void stage_edit::on_dialogs_line1_face_combo_currentIndexChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.face_graphics_filename, "%s", arg1.toStdString().c_str());
}

// ------------------- string tooltip buttons ------------------- //
// --- STAGE-DIALOG --- //
void stage_edit::on_phrase1_1_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.text1_string_ids[0]), ui->dialogs_line1_text1);
}

void stage_edit::on_phrase1_2_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.text1_string_ids[1]), ui->dialogs_line1_text2);
}

void stage_edit::on_phrase1_3_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.text1_string_ids[2]), ui->dialogs_line1_text3);
}

void stage_edit::on_phrase2_1_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.text2_string_ids[0]), ui->dialogs_line2_text1);
}

void stage_edit::on_phrase2_2_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.text2_string_ids[1]), ui->dialogs_line2_text2);
}

void stage_edit::on_phrase2_3_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.text2_string_ids[2]), ui->dialogs_line2_text3);
}

void stage_edit::on_answer1_1_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer1_string_ids[ui->dialogs_answer1_player->currentIndex()][0]), ui->dialogs_answer1_text1);
}

void stage_edit::on_answer1_2_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer1_string_ids[ui->dialogs_answer1_player->currentIndex()][1]), ui->dialogs_answer1_text2);
}

void stage_edit::on_answer1_3_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer1_string_ids[ui->dialogs_answer1_player->currentIndex()][2]), ui->dialogs_answer1_text3);
}

void stage_edit::on_answer2_1_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer2_string_ids[ui->dialogs_answer1_player->currentIndex()][0]), ui->dialogs_answer2_text1);
}

void stage_edit::on_answer2_2_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer2_string_ids[ui->dialogs_answer1_player->currentIndex()][1]), ui->dialogs_answer2_text2);
}

void stage_edit::on_answer2_3_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].intro_dialog.answer2_string_ids[ui->dialogs_answer1_player->currentIndex()][2]), ui->dialogs_answer2_text3);
}

// --- BOSS-DIALOG --- //

void stage_edit::on_boss_phrase1_1_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.text1_string_ids[0]), ui->boss_dialog_text1_line1);
}

void stage_edit::on_boss_phrase1_2_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.text1_string_ids[1]), ui->boss_dialog_text1_line2);
}

void stage_edit::on_boss_phrase1_3_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.text1_string_ids[2]), ui->boss_dialog_text1_line3);
}

void stage_edit::on_boss_phrase2_1_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.text2_string_ids[0]), ui->boss_dialog_text2_line1);
}

void stage_edit::on_boss_phrase2_2_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.text2_string_ids[1]), ui->boss_dialog_text2_line2);
}

void stage_edit::on_boss_phrase2_3_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.text2_string_ids[2]), ui->boss_dialog_text2_line3);
}

void stage_edit::on_boss_answer1_1_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer1_string_ids[ui->dialogs_answer1_player->currentIndex()][0]), ui->boss_dialog_answer1_line1);
}

void stage_edit::on_boss_answer1_2_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer1_string_ids[ui->dialogs_answer1_player->currentIndex()][1]), ui->boss_dialog_answer1_line2);
}

void stage_edit::on_boss_answer1_3_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer1_string_ids[ui->dialogs_answer1_player->currentIndex()][2]), ui->boss_dialog_answer1_line3);
}

void stage_edit::on_boss_answer2_1_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer2_string_ids[ui->dialogs_answer1_player->currentIndex()][0]), ui->boss_dialog_answer2_line1);
}

void stage_edit::on_boss_answer2_2_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer2_string_ids[ui->dialogs_answer1_player->currentIndex()][1]), ui->boss_dialog_answer2_line2);
}

void stage_edit::on_boss_answer2_3_toolButton_clicked()
{
    string_tooltip_click(&(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss_dialog.answer2_string_ids[ui->dialogs_answer1_player->currentIndex()][2]), ui->boss_dialog_answer2_line3);
}



void stage_edit::on_string_selected(int string_id)
{
    StringsEditor* strings_editor_window = (StringsEditor*)sender();
    int* value_property = strings_editor_window->get_target_property();
    *value_property = string_id;
    QLineEdit* qline = strings_editor_window->get_target_qline();
    qline->setText(QString(fio_str.get_common_string(string_id).c_str()));
}

void stage_edit::string_tooltip_click(int *property, QLineEdit *qline)
{
    StringsEditor* strings_editor_window = new StringsEditor(this, true);
    QObject::connect(strings_editor_window, SIGNAL(on_accepted(int)), this, SLOT(on_string_selected(int)));
    strings_editor_window->set_target_property(property);
    strings_editor_window->set_target_qline(qline);
    strings_editor_window->show();
}


void stage_edit::on_cutscenePre_comboBox_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].cutscene_pre = index;
}

void stage_edit::on_cutscenePos_comboBox_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].cutscene_pos = index;
}

void stage_edit::on_stages_tab_bossname_lineedit_textChanged(const QString &arg1)
{
    if (_data_loading == true) return;
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.name, "%s", arg1.toStdString().c_str());
}
