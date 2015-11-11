#include "gamescenes.h"
#include "ui_gamescenes.h"

GameScenes::GameScenes(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameScenes)
{
    ui->setupUi(this);
    data_loading = false;
}

GameScenes::~GameScenes()
{
    delete ui;
}

void GameScenes::on_company_combo_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_INTRO_COMPANY] = index;
}

void GameScenes::on_intro_combo_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_INTRO_GAME_UNBEATEN] = index;
}

void GameScenes::on_intro_beaten_combo_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_INTRO_GAME_BEATEN] = index;
}

void GameScenes::on_ending_player1_combo_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_ENDING_PLAYER1] = index;
}

void GameScenes::on_ending_player2_combo_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_ENDING_PLAYER2] = index;
}

void GameScenes::on_ending_player3_combo_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_ENDING_PLAYER3] = index;
}

void GameScenes::on_ending_player4_combo_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_ENDING_PLAYER4] = index;
}

void GameScenes::on_ending_credits_combo_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->game_scenes_map[GAME_SCENE_TYPES_ENDING_GAME_CREDITS] = index;
}
