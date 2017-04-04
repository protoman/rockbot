#include "gametextcreditstab.h"
#include "ui_gametextcreditstab.h"

#include "defines.h"
#include "file/file_io.h"
#include "file/fio_strings.h"
#include "aux_tools/stringutils.h"
#include "common.h"

extern std::string FILEPATH;

GameTextCreditsTab::GameTextCreditsTab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameTextCreditsTab)
{
    ui->setupUi(this);


    current_stage = 0;
    data_loading = true;
    common::fill_stages_combo(ui->stageSelect_comboBox);
    load_data();
    set_stage_data(INTRO_STAGE);
    data_loading = false;
}

GameTextCreditsTab::~GameTextCreditsTab()
{
    delete ui;
}

void GameTextCreditsTab::load_data()
{
    CURRENT_FILE_FORMAT::fio_strings fio_str;
    game_credits_data = fio_str.get_string_list_from_file(FILEPATH + "/game_credits.txt", LANGUAGE_ENGLISH);
    // read file and fill void spaces in boss credits with empty strings
    boss_credits_data = fio_str.get_string_list_from_file(FILEPATH + "/boss_credits.txt", LANGUAGE_ENGLISH);
    int max_lines = STAGE_COUNT * 3;
    if (boss_credits_data.size() < max_lines) {
        for (int i=boss_credits_data.size(); i<max_lines; i++) {
            boss_credits_data.push_back(std::string(""));
        }
    }

    ui->gameCredits_textEdit->clear();
    for (int i=0; i<game_credits_data.size(); i++) {
        ui->gameCredits_textEdit->append(QString(game_credits_data.at(i).c_str()));
    }
}

void GameTextCreditsTab::set_stage_data(int stage_n)
{
    std::cout << "change-stage, old[" << current_stage << "], new[" << stage_n << "]" << std::endl;
    current_stage = stage_n;
    ui->bossCredits_lineEdit1->setText(boss_credits_data.at(current_stage*3).c_str());
    ui->bossCredits_lineEdit2->setText(boss_credits_data.at(current_stage*3+1).c_str());
    ui->bossCredits_lineEdit3->setText(boss_credits_data.at(current_stage*3+2).c_str());
}

void GameTextCreditsTab::save_data()
{
    CURRENT_FILE_FORMAT::fio_strings fio_str;
    QString game_credits_text = ui->gameCredits_textEdit->toPlainText();
    game_credits_text.replace("\r", "");
    QStringList splitted_list = game_credits_text.split("\n");

    game_credits_data.clear();
    foreach(QString str, splitted_list) {
      game_credits_data.push_back(str.toStdString());
    }

    fio_str.write_string_list_to_file(game_credits_data, FILEPATH + "/game_credits.txt", LANGUAGE_ENGLISH);
    fio_str.write_string_list_to_file(boss_credits_data, FILEPATH + "/boss_credits.txt", LANGUAGE_ENGLISH);
}

void GameTextCreditsTab::on_bossCredits_lineEdit1_textChanged(const QString &arg1)
{
    if (data_loading) return;
    boss_credits_data.at(current_stage*3) = arg1.toStdString();
}

void GameTextCreditsTab::on_bossCredits_lineEdit2_textChanged(const QString &arg1)
{
    if (data_loading) return;
    boss_credits_data.at(current_stage*3+1) = arg1.toStdString();
}

void GameTextCreditsTab::on_bossCredits_lineEdit3_textChanged(const QString &arg1)
{
    if (data_loading) return;
    boss_credits_data.at(current_stage*3+2) = arg1.toStdString();
}

void GameTextCreditsTab::on_stageSelect_comboBox_currentIndexChanged(int index)
{
    if (data_loading) {
        return;
    }
    set_stage_data(index);
}
