#include "anim_tiles_edit.h"
#include "ui_anim_tiles_edit.h"

anim_tiles_edit::anim_tiles_edit(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::anim_tiles_edit)
{
    ui->setupUi(this);
}

anim_tiles_edit::~anim_tiles_edit()
{
    delete ui;
}
