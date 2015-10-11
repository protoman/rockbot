#include "gamescenes.h"
#include "ui_gamescenes.h"

GameScenes::GameScenes(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GameScenes)
{
    ui->setupUi(this);
}

GameScenes::~GameScenes()
{
    delete ui;
}
