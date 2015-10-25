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

void GameScenes::on_company_scene_currentIndexChanged(int index)
{
    if (data_loading) { return; }
    ScenesMediator::get_instance()->game_scenes_map[0] = index;
}
