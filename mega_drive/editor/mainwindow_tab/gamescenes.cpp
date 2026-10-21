#include "gamescenes.h"
#include "ui_gamescenes.h"
#include "common.h"

GameScenes::GameScenes(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameScenes)
{
    data_loading = true;
    ui->setupUi(this);
    fill_data();
    data_loading = false;
}

GameScenes::~GameScenes()
{
    delete ui;
}

void GameScenes::reload()
{
    data_loading = true;
    Mediator::get_instance()->reload_game_scenes();
    fill_data();
    data_loading = false;
}

void GameScenes::fill_data()
{
    common::fill_scenes_combo(ui->company_combo);
    common::fill_scenes_combo(ui->intro_combo);
    common::fill_scenes_combo(ui->intro_beaten_combo);

    common::fill_scenes_combo(ui->ending_player1_combo);
    common::fill_scenes_combo(ui->ending_player2_combo);
    common::fill_scenes_combo(ui->ending_player3_combo);
    common::fill_scenes_combo(ui->ending_player4_combo);

    common::fill_scenes_combo(ui->ending_credits_combo);

    ui->company_combo->setCurrentIndex(ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_INTRO_COMPANY]+1);
    ui->intro_combo->setCurrentIndex(ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_INTRO_GAME_UNBEATEN]+1);
    ui->intro_beaten_combo->setCurrentIndex(ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_INTRO_GAME_BEATEN]+1);

    ui->ending_player1_combo->setCurrentIndex(ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_ENDING_PLAYER1]+1);
    ui->ending_player2_combo->setCurrentIndex(ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_ENDING_PLAYER2]+1);
    ui->ending_player3_combo->setCurrentIndex(ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_ENDING_PLAYER3]+1);
    ui->ending_player4_combo->setCurrentIndex(ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_ENDING_PLAYER4]+1);

    ui->ending_credits_combo->setCurrentIndex(ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_ENDING_GAME_CREDITS]+1);
}

void GameScenes::on_company_combo_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_INTRO_COMPANY] = index-1;
}

void GameScenes::on_intro_combo_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_INTRO_GAME_UNBEATEN] = index-1;
}

void GameScenes::on_intro_beaten_combo_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_INTRO_GAME_BEATEN] = index-1;
}

void GameScenes::on_ending_player1_combo_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_ENDING_PLAYER1] = index-1;
}

void GameScenes::on_ending_player2_combo_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_ENDING_PLAYER2] = index-1;
}

void GameScenes::on_ending_player3_combo_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_ENDING_PLAYER3] = index-1;
}

void GameScenes::on_ending_player4_combo_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_ENDING_PLAYER4] = index-1;
}

void GameScenes::on_ending_credits_combo_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_ENDING_GAME_CREDITS] = index-1;
}
