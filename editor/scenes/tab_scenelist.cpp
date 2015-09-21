#include "sceneslist.h"
#include "ui_sceneslist.h"

ScenesList::ScenesList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ScenesList)
{
    ui->setupUi(this);
}

ScenesList::~ScenesList()
{
    delete ui;
}
