#include "tab_changecolorcycle.h"
#include "ui_tab_changecolorcycle.h"

TabChangeColorcycle::TabChangeColorcycle(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabChangeColorcycle)
{
    ui->setupUi(this);
}

TabChangeColorcycle::~TabChangeColorcycle()
{
    delete ui;
}
