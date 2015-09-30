#include "tab_cleararea.h"
#include "ui_tab_cleararea.h"

TabClearArea::TabClearArea(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabClearArea)
{
    ui->setupUi(this);
}

TabClearArea::~TabClearArea()
{
    delete ui;
}
