#include "animpackimport.h"
#include "ui_animpackimport.h"

AnimPackImport::AnimPackImport(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AnimPackImport)
{
    ui->setupUi(this);
}

AnimPackImport::~AnimPackImport()
{
    delete ui;
}
