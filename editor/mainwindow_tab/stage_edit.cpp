#include "stage_edit.h"
#include "ui_stage_edit.h"

#include "common.h"

#include "../mediator.h"


stage_edit::stage_edit(QWidget *parent) : QWidget(parent), ui(new Ui::stage_edit), _data_loading(true), initialized(false)
{

    Mediator::get_instance()->stage_dialog_list.clear();
    for (int i=0; i<LANGUAGE_COUNT; i++) {
        std::map<int, std::vector<std::string> > entry;
        Mediator::get_instance()->stage_dialog_list.push_back(entry);
    }
    Mediator::get_instance()->currentStage = INTRO_STAGE;


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

    common::fill_languages_combo(ui->language_comboBox);


    fill_stage_tab_data(ui->language_comboBox->currentIndex());
    _data_loading = false;
}

stage_edit::~stage_edit()
{
    delete ui;
}

void stage_edit::reload()
{
    Mediator::get_instance()->currentStage = INTRO_STAGE;
    fill_stage_tab_data(ui->language_comboBox->currentIndex());
}

void stage_edit::fill_stage_tab_data(int language_n)
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

    common::fill_scenes_combo(ui->cutscenePre_comboBox);

    common::fill_scenes_combo(ui->cutscenePos_comboBox);

    update_stage_data(language_n);

    _data_loading = false;
}


void stage_edit::update_stage_data(int language_n)
{
    if (Mediator::get_instance()->currentStage < 0) {
        return;
    }

	// SET ITEMS
    ui->stages_tab_stage_name_lineedit->setText(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].name);
    ui->stages_tab_bossname_lineedit->setText(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.name);
    int combo_n = ui->stages_tab_bgmusic_combo->findText(QString(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].bgmusic_filename));
    ui->stages_tab_bgmusic_combo->setCurrentIndex(combo_n);

    // search for the item that matches the text to select row
    std::string search_text = std::string(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].dialog_face_graphics_filename);
    for (int i=0; i<ui->dialogs_line1_face_combo->count(); i++) {
        int combo_n = ui->dialogs_line1_face_combo->findText(QString(search_text.c_str()));
        ui->dialogs_line1_face_combo->setCurrentIndex(combo_n);
        break;
    }

    int stage_id = Mediator::get_instance()->currentStage;
    if (language_n < 0 || language_n >= LANGUAGE_COUNT) {
        language_n = LANGUAGE_ENGLISH;
    }
    Mediator::get_instance()->stage_dialog_list[language_n].clear();
    // default language (english) is used as placeholder
    stage_dialog_default_language_list.insert(std::pair<int, std::vector<std::string> >(stage_id, fio_str.get_stage_dialogs(Mediator::get_instance()->currentStage, LANGUAGE_ENGLISH, false)));
    Mediator::get_instance()->stage_dialog_list[language_n].insert(std::pair<int, std::vector<std::string> >(stage_id, fio_str.get_stage_dialogs(Mediator::get_instance()->currentStage, language_n, false)));

    ui->dialogs_line1_text1->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(0).c_str()));
    ui->dialogs_line1_text2->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(1).c_str()));
    ui->dialogs_line1_text3->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(2).c_str()));
    ui->dialogs_line1_text1->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(0).c_str()));
    ui->dialogs_line1_text2->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(1).c_str()));
    ui->dialogs_line1_text3->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(2).c_str()));

    ui->dialogs_line2_text1->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(3).c_str()));
    ui->dialogs_line2_text2->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(4).c_str()));
    ui->dialogs_line2_text3->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(5).c_str()));
    ui->dialogs_line2_text1->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(3).c_str()));
    ui->dialogs_line2_text2->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(4).c_str()));
    ui->dialogs_line2_text3->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(5).c_str()));

    int player_adjust = ui->dialogs_answer1_player->currentIndex() * 6;
    ui->dialogs_answer1_text1->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(6+player_adjust).c_str()));
    ui->dialogs_answer1_text2->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(7+player_adjust).c_str()));
    ui->dialogs_answer1_text3->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(8+player_adjust).c_str()));
    ui->dialogs_answer1_text1->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(6+player_adjust).c_str()));
    ui->dialogs_answer1_text2->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(7+player_adjust).c_str()));
    ui->dialogs_answer1_text3->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(8+player_adjust).c_str()));

    ui->dialogs_answer2_text1->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(9+player_adjust).c_str()));
    ui->dialogs_answer2_text2->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(10+player_adjust).c_str()));
    ui->dialogs_answer2_text3->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(11+player_adjust).c_str()));
    ui->dialogs_answer2_text1->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(9+player_adjust).c_str()));
    ui->dialogs_answer2_text2->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(10+player_adjust).c_str()));
    ui->dialogs_answer2_text3->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(11+player_adjust).c_str()));

    // *********** BOSS DIALOGS *************** //
    int adjust_1 = 5+ 4*6; // 4 players, 6 lines each
    ui->boss_dialog_text1_line1->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(adjust_1+1).c_str()));
    ui->boss_dialog_text1_line2->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(adjust_1+2).c_str()));
    ui->boss_dialog_text1_line3->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(adjust_1+3).c_str()));
    ui->boss_dialog_text1_line1->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(adjust_1+1).c_str()));
    ui->boss_dialog_text1_line2->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(adjust_1+2).c_str()));
    ui->boss_dialog_text1_line3->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(adjust_1+3).c_str()));

    ui->boss_dialog_text2_line1->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(adjust_1+4).c_str()));
    ui->boss_dialog_text2_line2->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(adjust_1+5).c_str()));
    ui->boss_dialog_text2_line3->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(adjust_1+6).c_str()));
    ui->boss_dialog_text2_line1->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(adjust_1+4).c_str()));
    ui->boss_dialog_text2_line2->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(adjust_1+5).c_str()));
    ui->boss_dialog_text2_line3->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(adjust_1+6).c_str()));

    int adjust_boss_player = adjust_1+7 + ui->dialogs_answer1_player->currentIndex() * 6;
    ui->boss_dialog_answer1_line1->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(adjust_boss_player).c_str()));
    ui->boss_dialog_answer1_line2->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(adjust_boss_player+1).c_str()));
    ui->boss_dialog_answer1_line3->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(adjust_boss_player+2).c_str()));
    ui->boss_dialog_answer1_line1->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(adjust_boss_player).c_str()));
    ui->boss_dialog_answer1_line2->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(adjust_boss_player+1).c_str()));
    ui->boss_dialog_answer1_line3->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(adjust_boss_player+2).c_str()));


    ui->boss_dialog_answer2_line1->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(adjust_boss_player+3).c_str()));
    ui->boss_dialog_answer2_line2->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(adjust_boss_player+4).c_str()));
    ui->boss_dialog_answer2_line3->setText(QString(Mediator::get_instance()->stage_dialog_list[language_n].at(stage_id).at(adjust_boss_player+5).c_str()));
    ui->boss_dialog_answer2_line1->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(adjust_boss_player+3).c_str()));
    ui->boss_dialog_answer2_line2->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(adjust_boss_player+4).c_str()));
    ui->boss_dialog_answer2_line3->setPlaceholderText(QString(stage_dialog_default_language_list.at(stage_id).at(adjust_boss_player+5).c_str()));


    ui->cutscenePre_comboBox->setCurrentIndex(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].cutscene_pre);
    ui->cutscenePos_comboBox->setCurrentIndex(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].cutscene_pos);

    ui->stages_tab_bossname_lineedit->setText(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].boss.name);


    if (Mediator::get_instance()->stage_extra_data.extra_data[stage_id].active == true) {
        ui->checkBox->setCheckState(Qt::Checked);
    } else {
        ui->checkBox->setCheckState(Qt::Unchecked);
    }
}


void stage_edit::on_stages_tab_stage_combo_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->currentStage = index;
    Mediator::get_instance()->save_dialogs();
    _data_loading = true;
    update_stage_data(ui->language_comboBox->currentIndex());
    _data_loading = false;
}


void stage_edit::on_dialogs_answer1_player_currentIndexChanged(int index)
{
    if (_data_loading == true) return;
    Mediator::get_instance()->current_player = index;
    Mediator::get_instance()->save_dialogs();
    _data_loading = true;
    update_stage_data(ui->language_comboBox->currentIndex());
    _data_loading = false;
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
    sprintf(Mediator::get_instance()->stage_data.stages[Mediator::get_instance()->currentStage].dialog_face_graphics_filename, "%s", arg1.toStdString().c_str());
}




void stage_edit::on_string_selected(int string_id)
{
    StringsEditor* strings_editor_window = (StringsEditor*)sender();
    int* value_property = strings_editor_window->get_target_property();
    *value_property = string_id;
    QLineEdit* qline = strings_editor_window->get_target_qline();
    qline->setText(QString(fio_str.get_common_string(string_id, ui->language_comboBox->currentIndex(), false).c_str()));
}

void stage_edit::string_tooltip_click(int *property, QLineEdit *qline)
{
    StringsEditor* strings_editor_window = new StringsEditor(this, 1);
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

void stage_edit::on_dialogs_line1_text1_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(0) = arg1.toStdString();
}

void stage_edit::on_dialogs_line1_text2_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int stage_n = ui->stages_tab_stage_combo->currentIndex();
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(stage_n).at(1) = arg1.toStdString();
}

void stage_edit::on_dialogs_line1_text3_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(2) = arg1.toStdString();
}

void stage_edit::on_dialogs_line2_text1_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(3) = arg1.toStdString();
}

void stage_edit::on_dialogs_line2_text2_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(4) = arg1.toStdString();
}

void stage_edit::on_dialogs_line2_text3_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(5) = arg1.toStdString();
}

void stage_edit::on_dialogs_answer1_text1_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int player_adjust = ui->dialogs_answer1_player->currentIndex() * 6 + 6;
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(player_adjust) = arg1.toStdString();
}

void stage_edit::on_dialogs_answer1_text2_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int player_adjust = ui->dialogs_answer1_player->currentIndex() * 6 + 6;
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(player_adjust+1) = arg1.toStdString();
}

void stage_edit::on_dialogs_answer1_text3_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int player_adjust = ui->dialogs_answer1_player->currentIndex() * 6 + 6;
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(player_adjust+2) = arg1.toStdString();
}

void stage_edit::on_dialogs_answer2_text1_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int player_adjust = ui->dialogs_answer1_player->currentIndex() * 6 + 6;
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(player_adjust+3) = arg1.toStdString();
}

void stage_edit::on_dialogs_answer2_text2_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int player_adjust = ui->dialogs_answer1_player->currentIndex() * 6 + 6;
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(player_adjust+4) = arg1.toStdString();
}

void stage_edit::on_dialogs_answer2_text3_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int player_adjust = ui->dialogs_answer1_player->currentIndex() * 6 + 6;
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(player_adjust+5) = arg1.toStdString();
}

void stage_edit::on_boss_dialog_text1_line1_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int adjust_1 = 5+ 4*6; // 4 players, 6 lines each
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(adjust_1+1) = arg1.toStdString();
}

void stage_edit::on_boss_dialog_text1_line2_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int adjust_1 = 5+ 4*6; // 4 players, 6 lines each
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(adjust_1+2) = arg1.toStdString();
}

void stage_edit::on_boss_dialog_text1_line3_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int adjust_1 = 5+ 4*6; // 4 players, 6 lines each
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(adjust_1+3) = arg1.toStdString();
}

void stage_edit::on_boss_dialog_text2_line1_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int adjust_1 = 5+ 4*6; // 4 players, 6 lines each
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(adjust_1+4) = arg1.toStdString();
}

void stage_edit::on_boss_dialog_text2_line2_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int adjust_1 = 5+ 4*6; // 4 players, 6 lines each
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(adjust_1+5) = arg1.toStdString();
}

void stage_edit::on_boss_dialog_text2_line3_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int adjust_1 = 5+ 4*6; // 4 players, 6 lines each
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(adjust_1+6) = arg1.toStdString();
}

void stage_edit::on_boss_dialog_answer1_line1_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int adjust_boss_player = 5 + 4*6 +7 + ui->dialogs_answer1_player->currentIndex() * 6;
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(adjust_boss_player) = arg1.toStdString();
}

void stage_edit::on_boss_dialog_answer1_line2_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int adjust_boss_player = 5 + 4*6 +7 + ui->dialogs_answer1_player->currentIndex() * 6;
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(adjust_boss_player+1) = arg1.toStdString();
}

void stage_edit::on_boss_dialog_answer1_line3_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int adjust_boss_player = 5 + 4*6 +7 + ui->dialogs_answer1_player->currentIndex() * 6;
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(adjust_boss_player+2) = arg1.toStdString();
}

void stage_edit::on_boss_dialog_answer2_line1_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int adjust_boss_player = 5 + 4*6 +7 + ui->dialogs_answer1_player->currentIndex() * 6;
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(adjust_boss_player+3) = arg1.toStdString();
}

void stage_edit::on_boss_dialog_answer2_line2_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int adjust_boss_player = 5 + 4*6 +7 + ui->dialogs_answer1_player->currentIndex() * 6;
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(adjust_boss_player+4) = arg1.toStdString();
}

void stage_edit::on_boss_dialog_answer2_line3_textChanged(const QString &arg1)
{
    if (_data_loading) { return; }
    int adjust_boss_player = 5 + 4*6 +7 + ui->dialogs_answer1_player->currentIndex() * 6;
    Mediator::get_instance()->stage_dialog_list[ui->language_comboBox->currentIndex()].at(ui->stages_tab_stage_combo->currentIndex()).at(adjust_boss_player+5) = arg1.toStdString();
}

void stage_edit::on_language_comboBox_currentIndexChanged(int index)
{
    if (_data_loading) { return; }
    Mediator::get_instance()->save_dialogs();
    update_stage_data(index);
}

void stage_edit::on_checkBox_stateChanged(int arg1)
{
    if (_data_loading) { return; }
    bool active = false;
    if (arg1 == Qt::Checked) {
        active = true;
    }
    Mediator::get_instance()->stage_extra_data.extra_data[Mediator::get_instance()->currentStage].active = active;
}
